/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file pmem.c
 * Physical memory subsystem. Allocates physical memory pages, with support for
 * different ordered pages, depending on the underlying architecture.
 *
 * Quick overview of the physical memory subsystem: Somewhere in RAM there
 * exists a number of buckets, each with an n-tree representing different order
 * pages and their status (used/free). When a lower-order memory page (i.e.
 * smaller) is allocated, it blocks allocation of higher-order pages (i.e.
 * larger) whose addresses would overlap. This is avoided by marking all
 * higher-order pages as used in their respective buckets.
 *
 * This approach is reasonably efficient at handling the different possible page
 * sizes, but requires that the caller maintains some data about page sizes, as
 * the algorithm doesn't keep any of that information. Allocating a region of a
 * certain page order and freeing it as another could easily be a
 * source of difficult to track bugs.
 *
 * \todo More in depth documentation about the physical memory algorithms,
 * unfortunately it is quite difficult to follow.
 *
 * \todo See if there are improvements to be made, either to the implementation
 * or code in general. Could I use bitmaps, for example, and maybe calculate the
 * next pointer instead of storing it?
 */

#include <kmi/mem_nodes.h>
#include <kmi/pmem.h>
#include <kmi/dmem.h>
#include <kmi/debug.h>
#include <kmi/initrd.h>
#include <kmi/string.h> /* memset */
#include <kmi/bits.h> /* is_nset etc */
#include <libfdt.h>

/* \todo add memory page counting?
 * To make sure memory is not overcommited at clone, for example.
 */

/**
 * Beauty macro for looping over all page indexes.
 * The current page index is stored in \c page.
 *
 * @param num Number of pages in branch.
 */
#define foreach_page(num) \
	for (pm_t page = 0; page < num; ++page)

/**
 * Loop over orders, giving the iterator the name \p iter.
 *
 * @param iter Name of iterator.
 */
#define foreach_order(iter) \
	for (enum mm_order iter = MM_O0; iter <= max_order(); ++iter)

/**
 * Loop over orders, with already initialized start iterator \p iter.
 *
 * @param iter Name of iterator.
 */
#define foreach_order_init(iter) \
	for (; iter <= max_order(); ++iter)

/**
 * Loop over orders in reverse, starting with highest, giving the iterator the
 * name \p iter.
 *
 * @param iter Name of iterator.
 */
#define reverse_foreach_order(iter) \
	for (enum mm_order iter = max_order(); iter != MM_MIN; --iter)

/**
 * Loop over orders in reverse, starting with highest, with already initialized
 * start iterator \p iter.
 *
 * @param iter Name of iterator.
 */
#define reverse_foreach_order_init(iter) \
	for (; iter != MM_MIN; --iter)

/** Beauty typedef for uint8_t *, used for bitmaps in this file. */
typedef uint8_t mm_bitmap_t[];

/** Memory page branch. */
struct mm_branch {
	/** Number of entries in leaf. */
	size_t num;

	/** Size of one whole span of one sub branch. */
	size_t size;

	/** Bitmap of used pages. */
	mm_bitmap_t used;
};

/** Order map. */
struct mm_bucket {
	/** Base address of map. */
	pm_t base;

	/** Order of map. */
	enum mm_order order;

	/** Pointer to array of nodes. */
	struct mm_branch *tree[MM_NUM];
};

/** Physical map. */
struct mm_pmap {
	/** Buckets, one per order up to maximum order. */
	struct mm_bucket *bucket[NUM_ORDERS];
};

/** Static physical map address. \note If I support NUMA, this should not be
 * static, rather one physical map per NUMA region. */
static struct mm_pmap *pmap = 0;

/**
 * Calculate size of branch structure plus bitmap for branch.
 *
 * @param num Number of elements in branch.
 * @return Size in bytes of a branch.
 */
static size_t sizeof_branch(size_t num)
{
	return align_up(sizeof(struct mm_branch) + (num + 8) / 8,
	                sizeof(struct mm_branch));
}

/**
 * Get top of current branch, that is, the start of a following branch.
 *
 * @param branch Branch whose top to calculate.
 * @return Top of \p branch.
 */
static struct mm_branch *branch_top(struct mm_branch *branch)
{
	return (struct mm_branch *)(sizeof_branch(branch->num) + (pm_t)branch);
}

/**
 * Get the branch under \p branch at \p index.
 *
 * @param branch Branch whose sub branches to access.
 * @param index Index of sub branch to access.
 * @return Pointer to sub branch.
 */
static struct mm_branch *sub_branch(struct mm_branch *branch, size_t index)
{
	struct mm_branch *sub_start = branch_top(branch);
	return (struct mm_branch *)(index * branch->size + (pm_t)sub_start);
}

/**
 * Mark a page free in tree.
 *
 * @param branch Branch wherein some part of \p page lies.
 * @param page Page address relative to start of RAM.
 * @param req_order Order of page to be marked free.
 * @param cur_order Current page order.
 * @param tree_order Order context we're in.
 */
static void __mark_free(struct mm_branch *branch, pm_t page,
                        enum mm_order req_order,
                        enum mm_order cur_order,
                        enum mm_order tree_order)
{
	size_t idx = pm_to_index(page, cur_order);

	if (cur_order == req_order) {
		bitmap_clear(branch->used, idx);
		return;
	}

	if(cur_order != tree_order)
		__mark_free(sub_branch(branch, idx), page,
		            req_order, cur_order - 1, tree_order);

	/* freeing a page results in always clearing a full bit */
	bitmap_clear(branch->used, idx);
}

/**
 * Mark page in bucket free in all trees.
 *
 * @param bucket Bucket page lies in.
 * @param order Order of page to free.
 * @param addr Physical address of page.
 */
static void __mark_bucket_page_free(struct mm_bucket *bucket,
                                    enum mm_order order, pm_t addr)
{
	pm_t fixup_addr = addr - bucket->base;
	enum mm_order iter = bucket->order;

	reverse_foreach_order_init(iter) {
		__mark_free(bucket->tree[iter], fixup_addr,
		            order, bucket->order, iter);
	}
}

void free_page(enum mm_order order, pm_t addr)
{
	foreach_order(iter) {
		struct mm_bucket *bucket = pmap->bucket[iter];
		if (!bucket)
			continue;

		if (addr < bucket->base)
			continue;

		__mark_bucket_page_free(bucket, order, addr);
		return;
	}
}

/**
 * Mark page used in tree.
 *
 * @param branch Current branch.
 * @param page Page address relative to start of RAM.
 * @param req_order Page order.
 * @param cur_order Current order.
 * @param tree_order Order of context we're in.
 * @return Whether the branch below got filled up.
 */
static bool __mark_used(struct mm_branch *branch, pm_t page,
                        enum mm_order req_order,
                        enum mm_order cur_order,
                        enum mm_order tree_order)
{
	size_t idx = pm_to_index(page, cur_order);

	if (cur_order == req_order || cur_order == tree_order) {
		bitmap_set(branch->used, idx);

		if (idx == max_index(cur_order))
			return true;

		return false;
	}

	bool r = __mark_used(sub_branch(branch, idx), page,
	                     req_order,
	                     cur_order - 1,
	                     tree_order);

	if (r) {
		bitmap_set(branch->used, idx);

		if (idx == max_index(cur_order))
			return true;
	}

	return false;
}

/**
 * Mark page in bucket used.
 *
 * @param bucket Bucket in which \p page lies.
 * @param order Order of \p page.
 * @param addr Physical address of \p page.
 */
static void __mark_bucket_page_used(struct mm_bucket *bucket,
                                    enum mm_order order, pm_t addr)
{
	pm_t fixed_addr = addr - bucket->base;
	reverse_foreach_order(iter) {
		__mark_used(bucket->tree[iter], fixed_addr,
		            order, bucket->order, iter);
	}
}

void mark_used(enum mm_order order, pm_t addr)
{
	enum mm_order iter = order;
	foreach_order_init(iter) {
		struct mm_bucket *bucket = pmap->bucket[iter];
		if (!bucket)
			continue;

		if (addr < bucket->base)
			continue;

		__mark_bucket_page_used(bucket, iter, addr);
		return;
	}
}

/**
 * Find first unused page on branch.
 * Helper for converting between bitmap and pmem error conditions.
 *
 * @param branch Branch to look for unused pages on.
 * @return \c -1 if there are no free pages, otherwise the index of first unused
 * page.
 */
static pm_t __branch_find_first_unset(struct mm_branch *branch)
{
	size_t r = bitmap_find_first_unset(branch->used, branch->num);
	if (r > branch->num)
		return -1;

	return r;
}

/**
 * Search for unused pages in tree.
 *
 * @param branch Current branch.
 * @param cur_order Current order of branch.
 * @param req_order Requested page order.
 * @return \c -1 if there are no free pages, otherwise the address of the lower
 * order page found.
 */
static pm_t __search_tree(struct mm_branch *branch,
                          enum mm_order cur_order,
                          enum mm_order req_order)
{
	pm_t page = __branch_find_first_unset(branch);
	if (page == (pm_t)(-1))
		return -1;

	if (cur_order == req_order)
		return page << order_shift(cur_order);

	pm_t r = __search_tree(sub_branch(branch, page),
	                       cur_order - 1, req_order);

	if (r == (pm_t)(-1))
		return -1;

	return (page << order_shift(cur_order)) + r;
}

pm_t alloc_page(enum mm_order order)
{
	if (order > max_order())
		return 0;

	pm_t p = -1;
	struct mm_bucket *bucket;
	enum mm_order iter = order;
	foreach_order_init(iter) {
		bucket = pmap->bucket[iter];
		if (!bucket)
			continue;

		p = __search_tree(bucket->tree[order], bucket->order, order);

		if (p != (pm_t)(-1))
			break;
	}

	if (p == (pm_t)(-1))
		return 0;

	p = p + bucket->base;
	__mark_bucket_page_used(bucket, order, p);
	return p;
}

/**
 * Populate tree.
 *
 * @param cont Address at which to continue placing data.
 * @param num Number of elements in branch.
 * @param cur_order Current branch order.
 * @param req_order Requested tree order.
 * @return Top of tree.
 */
static pm_t __populate_tree(pm_t cont, size_t num,
                            enum mm_order cur_order, enum mm_order req_order)
{
	size_t s = sizeof_branch(num);
	struct mm_branch *branch = (struct mm_branch *)cont;
	memset(branch, 0, s);
	cont += s;

	branch->num = num;
	if (cur_order == req_order)
		return cont;

	pm_t prev = cont;
	foreach_page(num) {
		prev = cont;
		cont = __populate_tree(cont, order_width(cur_order - 1),
		                       cur_order - 1, req_order);
	}

	branch->size = cont - prev;
	return cont;
}

/**
 * Populate bucket.
 *
 * @param cont Address at which to continue placing data.
 * @param base Base of bucket.
 * @param num Number of elements in top level trees.
 * @param order Bucket order.
 * @return Top of bucket.
 */
static pm_t __populate_bucket(pm_t cont, pm_t base, size_t num,
                              enum mm_order order)
{
	struct mm_bucket *bucket = (struct mm_bucket *)cont;
	memset(bucket, 0, sizeof(*bucket));
	cont += sizeof(*bucket);

	bucket->order = order;
	bucket->base = base;

	enum mm_order iter = order;
	reverse_foreach_order_init(iter) {
		bucket->tree[iter] = (struct mm_branch *)cont;
		cont = __populate_tree(cont, num, order, iter);
	}

	return cont;
}

pm_t populate_pmap(pm_t ram_base, size_t ram_size, pm_t cont)
{
	pm_t start = cont;

	pmap = (struct mm_pmap *)cont;
	memset(pmap, 0, sizeof(*pmap));
	cont += sizeof(*pmap);

	reverse_foreach_order(iter) {
		size_t num = ram_size / order_size(iter);
		if (num == 0)
			continue;

		pmap->bucket[iter] = (struct mm_bucket *)cont;
		cont = __populate_bucket(cont, ram_base, num, iter);

		ram_size -= order_size(iter) * num;
		ram_base += order_size(iter) * num;
	}

	return cont - start;
}

/**
 * Probe tree size.
 *
 * @param cont Size to continue adding to.
 * @param num Number of elements in tree.
 * @param cur_order Current branch order.
 * @param req_order Requested tree order.
 * @return Size of tree added to \p cont.
 */
static pm_t __probe_tree(pm_t cont, size_t num, enum mm_order cur_order,
                         enum mm_order req_order)
{
	cont += sizeof_branch(num);

	if (cur_order == req_order)
		return cont;

	foreach_page(num) {
		cont = __probe_tree(cont, order_width(cur_order - 1),
		                    cur_order - 1, req_order);
	}

	return cont;
}

/**
 * Probe bucket size.
 *
 * @param cont Size to continue adding to.
 * @param num Number of elements in bucket.
 * @param order Bucket order.
 * @return Size of bucket added to \p cont.
 */
static pm_t __probe_bucket(pm_t cont, size_t num, enum mm_order order)
{
	cont += sizeof(struct mm_bucket);

	reverse_foreach_order(iter) {
		cont = __probe_tree(cont, num, order, iter);
	}

	return cont;
}

pm_t probe_pmap(size_t ram_size)
{
	pm_t cont = sizeof(struct mm_pmap);

	reverse_foreach_order(iter) {
		size_t num = ram_size / order_size(iter);
		if (num == 0)
			continue;

		cont = __probe_bucket(cont, num, iter);

		ram_size -= order_size(iter) * num;
	}

	return cont;
}

/**
 * Helper function for marking area used.
 *
 * @param base Base address of area.
 * @param top Top address of top.
 */
static void __mark_area_used(pm_t base, pm_t top)
{
	size_t area_left = top - base;
	pm_t runner = base;
	while (area_left >= BASE_PAGE_SIZE) {
		mark_used(BASE_PAGE, runner);
		runner += BASE_PAGE_SIZE;
		area_left -= BASE_PAGE_SIZE;
	}

	if (area_left != 0)
		mark_used(BASE_PAGE, runner);
}

/**
 * Mark reserved memory region used, to avoid it getting accidentally allocated.
 *
 * @param fdt Global FDT pointer.
 */
static void __mark_reserved_mem(void *fdt)
{
	int rmem_offset = fdt_path_offset(fdt, "/reserved-memory/mmode_resv0");
	struct cell_info ci = get_reginfo(fdt, "/reserved-memory/mmode_resv0");
	uint8_t *rmem_reg =
		(uint8_t *)fdt_getprop(fdt, rmem_offset, "reg", NULL);

	pm_t base = (pm_t)fdt_load_int_ptr(ci.addr_cells, rmem_reg);

	if (ci.addr_cells == 2)
		rmem_reg += sizeof(fdt64_t);
	else
		rmem_reg += sizeof(fdt32_t);

	pm_t top = (pm_t)fdt_load_int_ptr(ci.size_cells, rmem_reg) + base;
	__mark_area_used((pm_t)__va(base), (pm_t)__va(top));
}

/**
 * Read top of RAM from FDT.
 *
 * @param fdt Global FDT pointer.
 * @return Physical address of top of RAM.
 */
static pm_t __get_ramtop(void *fdt)
{
	struct cell_info ci = get_reginfo(fdt, "/memory");
	int mem_offset = fdt_path_offset(fdt, "/memory");
	uint8_t *mem_reg = (uint8_t *)fdt_getprop(fdt, mem_offset, "reg", NULL);

	pm_t base = (pm_t)fdt_load_int_ptr(ci.addr_cells, mem_reg);

	if (ci.addr_cells == 2)
		mem_reg += sizeof(fdt64_t);
	else
		mem_reg += sizeof(fdt32_t);

	return (pm_t)fdt_load_int_ptr(ci.size_cells, mem_reg) + base;
}

/**
 * Read top of FDT.
 *
 * @param fdt Global FDT pointer.
 * @return Physical address of top of FDT.
 */
static pm_t __get_fdttop(void *fdt)
{
	const char *b = (const char *)fdt;
	return (pm_t)(b + fdt_totalsize(fdt));
}

/**
 * Return base of FDT.
 *
 * Technically pretty useless, but here mainly for cohesion.
 *
 * @param fdt Global FDT pointer.
 * @return \c fdt.
 */
static pm_t __get_fdtbase(void *fdt)
{
	/* lol */
	return (pm_t)fdt;
}

void init_pmem(void *fdt)
{
	size_t max_order = 0;
	size_t base_bits = 0;
	size_t bits[NUM_ORDERS] = { 0 };
	stat_pmem_conf(fdt, &max_order, &base_bits, bits);
	init_mem(max_order, bits, base_bits);

	pm_t ram_size = __get_ramtop(fdt) - get_ram_base();
	pm_t ram_base = (pm_t)__va(get_ram_base());

	pm_t initrd_top = get_initrdtop(fdt);
	pm_t fdt_top = __get_fdttop(fdt);

	/* find probably most suitable contiguous region of ram for our physical
	 * ram map */
	pm_t pmap_base = align_up(MAX(initrd_top, fdt_top), sizeof(int));

	size_t probe_size = probe_pmap(ram_size);
	size_t actual_size = populate_pmap(ram_base, ram_size, pmap_base);

	if (probe_size != actual_size) {
		bug("probe_size (%#lx) != actual_size (%#lx)\n", probe_size,
		    actual_size);
	}

	/* mark init stack, this should be unmapped once we get to executing
	 * processes */
	__mark_area_used(VM_STACK_BASE, VM_STACK_TOP);

	/* mark kernel */
	/* this could be made more explicit, I suppose. */
	__mark_area_used(VM_KERN, VM_KERN + PM_KERN_SIZE);

	/* mark fdt and initrd */
	__mark_area_used(get_initrdbase(fdt), initrd_top);
	__mark_area_used(__get_fdtbase(fdt), fdt_top);

	/* mark pmap */
	__mark_area_used(pmap_base, pmap_base + actual_size);

	/* mark reserved mem */
	__mark_reserved_mem(fdt);

	init_mem_nodes();

	init_devmem((pm_t)__pa(ram_base), (pm_t)__pa(ram_base + ram_size));
}
