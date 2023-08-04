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

	/* freeing a page results in always clearing a full bit */
	bitmap_clear(branch->used, idx);
	if(cur_order != tree_order)
		__mark_free(sub_branch(branch, idx), page,
		            req_order, cur_order - 1, tree_order);
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
		struct mm_branch *tree = bucket->tree[iter];
		if (!tree)
			continue;

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
		struct mm_branch *tree = bucket->tree[iter];
		if (!tree)
			continue;

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
 * Main worker for searching a free page.
 * Passing the address worked up so far allows the compiler to performa
 * tail call optimization, speeding things up a little.
 *
 * @param branch Current branch.
 * @param cur_order Current order of branch.
 * @param req_order Requested page order.
 * @param addr Address built up so far.
 * @return Final address of found free page if found, -1 otherwise.
 */
static pm_t __search_branch(struct mm_branch *branch,
                            enum mm_order cur_order,
                            enum mm_order req_order,
                            pm_t addr)
{
	pm_t page = __branch_find_first_unset(branch);
	if (page == (pm_t)(-1))
		return -1;

	if (cur_order == req_order)
		return addr | page << order_shift(cur_order);

	return __search_branch(sub_branch(branch, page),
	                       cur_order - 1, req_order,
	                       addr | page << order_shift(cur_order));
}

/**
 * Search for unused pages in tree.
 * Easy wrapper for __search_branch.
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
	return __search_branch(branch, cur_order, req_order, 0);
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
 * Zero out memory at \p cont if \p populate.
 *
 * @param populate Whether to populate at \p cont.
 * @param cont Where to zero out memory.
 * @param size How many bytes to zero.
 * @return \code cont + size \endcode
 */
static pm_t __zero_if(bool populate, pm_t cont, size_t size)
{
	if (populate)
		memset((void *)cont, 0, size);

	return cont + size;
}

/**
 * Populate tree.
 *
 * @param populate Whether to populate map or just probe.
 * @param cont Address at which to continue placing data.
 * @param num Number of elements in branch.
 * @param cur_order Current branch order.
 * @param req_order Requested tree order.
 * @return Top of tree.
 */
static pm_t __populate_tree(bool populate, pm_t cont, size_t num,
                            enum mm_order cur_order, enum mm_order req_order)
{
	struct mm_branch *branch = (struct mm_branch *)cont;
	cont = __zero_if(populate, cont, sizeof_branch(num));

	if (populate)
		branch->num = num;

	if (cur_order == req_order)
		return cont;

	pm_t prev = cont;
	foreach_page(num) {
		prev = cont;
		cont = __populate_tree(populate, cont,
		                       order_width(cur_order - 1),
		                       cur_order - 1, req_order);
	}

	if (populate)
		branch->size = cont - prev;

	return cont;
}

/**
 * Populate bucket.
 *
 * @param populate Whether to actually populate or just probe.
 * @param cont Address at which to continue placing data.
 * @param base Base of bucket.
 * @param num Number of elements in top level trees.
 * @param order Bucket order.
 * @return Top of bucket.
 */
static pm_t __populate_bucket(bool populate, pm_t cont, pm_t base, size_t num,
                              enum mm_order order)
{
	struct mm_bucket *bucket = (struct mm_bucket *)cont;
	cont = __zero_if(populate, cont, sizeof(*bucket));

	if (populate) {
		bucket->order = order;
		bucket->base = base;
	}

	enum mm_order iter = order;
	reverse_foreach_order_init(iter) {
		if (populate)
			bucket->tree[iter] = (struct mm_branch *)cont;

		cont = __populate_tree(populate, cont, num, order, iter);
	}

	return cont;
}

/**
 * Main worker for populating and probing the memory map.
 *
 * @param populate Whether to populate the map of just probe.
 * @param ram_base Base physical address of RAM.
 * @param ram_size Size of physical RAM.
 * @param cont Where to place the map.
 * @return Size of physical map.
 */
static size_t __populate_pmap(bool populate, pm_t ram_base, size_t ram_size,
                              pm_t cont)
{
	pm_t start = cont;

	pmap = (struct mm_pmap *)cont;
	cont = __zero_if(populate, cont, sizeof(*pmap));

	reverse_foreach_order(iter) {
		size_t num = ram_size / order_size(iter);
		if (num == 0)
			continue;

		if (populate)
			pmap->bucket[iter] = (struct mm_bucket *)cont;

		cont = __populate_bucket(populate, cont, ram_base, num, iter);

		ram_size -= order_size(iter) * num;
		ram_base += order_size(iter) * num;
	}

	return cont - start;
}

size_t populate_pmap(pm_t ram_base, size_t ram_size, pm_t cont)
{
	return __populate_pmap(true, ram_base, ram_size, cont);
}

size_t probe_pmap(pm_t ram_base, size_t ram_size, pm_t cont)
{
	return __populate_pmap(false, ram_base, ram_size, cont);
}

/**
 * Helper function for marking area used.
 *
 * @param base Base address of area.
 * @param top Top address of top.
 */
static void __mark_area_used(pm_t base, pm_t top)
{
	if (top < base) {
		bug("top < base: %lx < %lx\n", top, base);
		return;
	}

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
	int rmem_offset = fdt_path_offset(fdt, "/reserved-memory");
	struct cell_info ci = get_cellinfo(fdt, rmem_offset);

	int node = 0;
	fdt_for_each_subnode(node, fdt, rmem_offset) {
		uint8_t *rmem_reg =
			(uint8_t *)fdt_getprop(fdt, node, "reg", NULL);

		pm_t base = (pm_t)fdt_load_reg_addr(ci, rmem_reg, 0);

		/** @todo make sure the top of a reserved memory area doesn't go
		 * against our assumptions in FW_MAX_SIZE? */
		pm_t top = (pm_t)fdt_load_reg_size(ci, rmem_reg, 0) + base;
		__mark_area_used((pm_t)__va(base), (pm_t)__va(top));
		info("marked [%lx - %lx] reserved\n",
		     (pm_t)__va(base), (pm_t)__va(top));
	}
}

/**
 * Read top of RAM from FDT.
 *
 * @param fdt Global FDT pointer.
 * @return Physical address of top of RAM.
 */
static pm_t __get_ramtop(void *fdt)
{
	int mem_offset = fdt_path_offset(fdt, "/memory");
	const void *mem_reg = fdt_getprop(fdt, mem_offset, "reg", NULL);

	/* here we actually want the root offset because /memory itself doesn't
	 * have children, I guess? */
	struct cell_info ci = get_cellinfo(fdt, fdt_path_offset(fdt, "/"));
	pm_t base = (pm_t)fdt_load_reg_addr(ci, mem_reg, 0);
	return (pm_t)fdt_load_reg_size(ci, mem_reg, 0) + base;
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
	/** @todo should I keep the info outputs? I suppose it's nice to see
	 * if any assumption is being broken in the serial log, but in that case
	 * I should really try adding more of them to other parts of the
	 * codebase as well, the pmem subsystem isn't really especially complex.
	 */
	info("initializing pmem\n");

	size_t max_order = 0;
	size_t base_bits = 0;
	size_t bits[NUM_ORDERS] = { 0 };
	stat_pmem_conf(fdt, &max_order, &base_bits, bits);
	init_mem(max_order, bits, base_bits);

	pm_t ram_size = __get_ramtop(fdt) - get_ram_base();
	pm_t ram_base = (pm_t)__va(get_ram_base());

	info("using ram range [%lx - %lx]\n",
	     ram_base, ram_base + ram_size);

	/** @todo could probably improve error messages on failing to get fdt
	 * values */
	pm_t initrd_base = get_initrdbase(fdt);
	pm_t initrd_top = get_initrdtop(fdt);
	info("found initrd at [%lx - %lx]\n", initrd_base, initrd_top);

	pm_t fdt_top = __get_fdttop(fdt);
	pm_t fdt_base = __get_fdtbase(fdt);
	info("found fdt at [%lx - %lx]\n", fdt_base, fdt_top);

	/* find probably most suitable contiguous region of ram for our physical
	 * ram map  */
	/** @todo this could be better? */
	pm_t pmap_base = align_up(MAX(initrd_top, fdt_top), sizeof(int));
	info("choosing to place pmem map at %lx\n", pmap_base);

	size_t probe_size = probe_pmap(ram_base, ram_size, pmap_base);
	info("pmem map probe size returned %lu\n", probe_size);

	size_t actual_size = populate_pmap(ram_base, ram_size, pmap_base);
	info("pmem map actual size %lu\n", actual_size);

	if (probe_size != actual_size) {
		bug("probe_size (%#lx) != actual_size (%#lx)\n", probe_size,
		    actual_size);
	}

	/* mark init stack, this should be unmapped once we get to executing
	 * processes */
	__mark_area_used(VM_STACK_BASE, VM_STACK_TOP);
	info("marked stack [%lx - %lx] used\n", VM_STACK_BASE, VM_STACK_TOP);

	/* mark kernel */
	/* this could be made more explicit, I suppose. */
	__mark_area_used(VM_KERN, VM_KERN + PM_KERN_SIZE);
	info("marked kernel [%lx - %lx] used\n", VM_KERN,
	     VM_KERN + PM_KERN_SIZE);

	/* mark fdt and initrd */
	__mark_area_used(initrd_base, initrd_top);
	info("marked initrd [%lx - %lx] used\n", initrd_base, initrd_top);

	__mark_area_used(fdt_base, fdt_top);
	info("marked fdt [%lx - %lx] used\n", fdt_base, fdt_top);

	/* mark pmap */
	__mark_area_used(pmap_base, pmap_base + actual_size);
	info("marked pmap [%lx - %lx] used\n", pmap_base,
	     pmap_base + actual_size);

	/* mark reserved mem */
	__mark_reserved_mem(fdt);

	init_mem_nodes();

	init_devmem((pm_t)__pa(ram_base), (pm_t)__pa(ram_base + ram_size));
}
