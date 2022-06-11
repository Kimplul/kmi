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

#include <apos/mem_nodes.h>
#include <apos/pmem.h>
#include <apos/dmem.h>
#include <apos/debug.h>
#include <apos/initrd.h>
#include <apos/string.h> /* memset */
#include <apos/bits.h> /* is_nset etc */
#include <libfdt.h>

/**
 * Loop through all page usage bits in current bitmap.
 *
 * @param var Memory leaf or branch containing bitmap.
 * @param start Start looking from this index.
 * @param end Stop looking before this index.
 * @param attr Attribute name of bitmap.
 * @param neg Negate whether we're looking for full or empty pages.
 *
 * \note These are all for pnum_t, i.e. O0_SHIFT is from 0
 */
#define __foreach_page(var, start, end, attr, neg)          \
	for (pnum_t page = start; page < end; ++page)       \
	if (neg (bitmap_is_set(var->attr, page))) continue; \
	else                                                \

/** Easier to read negation. */
#define NEG !

/**
 * Loop through all full pages.
 *
 * @param var Memory leaf or branch containing bitmap.
 * @param start Start looking from this index.
 */
#define foreach_full_page(var, start) \
	__foreach_page(var, start, var->entries, full, NEG)

/**
 * Loop through all not full pages.
 *
 * @param var Memory leaf or branch containing bitmap.
 * @param start Start looking from this index.
 */
#define foreach_not_full_page(var, start) \
	__foreach_page(var, start, var->entries, full, )

/**
 * Loop through all used pages.
 *
 * @param var Memory leaf or branch containing bitmap.
 * @param start Start looking from this index.
 */
#define foreach_used_page(var, start) \
	__foreach_page(var, start, var->entries, used, NEG)

/**
 * Loop through all not used pages.
 *
 * @param var Memory leaf or branch containing bitmap.
 * @param start Start looking from this index.
 */
#define foreach_not_used_page(var, start) \
	__foreach_page(var, start, var->entries, used, )

/* curiously, all my optimisation efforts were in vain, and eight bits is the
 * best alternative. */

/** Memory bitmap base size. */
typedef uint8_t mm_info_t;

/** Beauty typedef for void *, used for bitmaps in this file. */
typedef void mm_node_t;

/** Memory page leaf. */
struct mm_leaf {
	/** Number of entries in leaf. */
	pnum_t entries;

	/** Bitmap of used pages. */
	mm_info_t *used;
};

/** Memory page branch. */
struct mm_branch {
	/** Number of entries in branch. */
	pnum_t entries;

	/** Bitmap of full nodes. */
	mm_info_t *full;

	/** Pointer to array of next order indexes. */
	mm_node_t **next;
};

/** Order map. */
struct mm_omap {
	/** Base address of map. */
	pm_t base;

	/** Pointer to array of nodes. */
	mm_node_t **orders;

	/** Order of map. */
	enum mm_order order;
};

/** Physical map. */
struct mm_pmap {
	/** Order map, one per order up to maximum order. */
	struct mm_omap *omap[NUM_ORDERS];
};

/** Static physical map address. \note If I support NUMA, this should not be
 * static, rather one physical map per NUMA region. */
static struct mm_pmap *pmap = 0;

/**
 * Helper function for marking a page used.
 *
 * @param op Order node pointer.
 * @param pnum Physical page number to mark free.
 * @param tgt Target order.
 * @param src Source order.
 * @param dst Destination order.
 */
static void __mark_free(mm_node_t *op, pnum_t pnum, enum mm_order tgt,
                        enum mm_order src, enum mm_order dst)
{
	size_t idx = pnum_to_index(pnum, src);

	if (src == dst) {
		struct mm_leaf *o = (struct mm_leaf *)op;
		bitmap_clear(o->used, idx);
		return;
	}

	struct mm_branch *o = (struct mm_branch *)op;
	if (src != tgt)
		__mark_free(o->next[idx], pnum, tgt, src - 1, dst);

	/* freeing a page results in always clearing a full bit? */
	bitmap_clear(o->full, idx);
}

void free_page(enum mm_order order, pm_t paddr)
{
	/** \todo This could probably use an int for status, but eh */
	for (size_t i = MM_O0; i <= __mm_max_order; ++i) {
		if (!pmap->omap[i])
			continue;

		struct mm_omap *omap = pmap->omap[i];
		if (paddr < omap->base)
			continue;

		for (size_t j = 0; j < omap->order; ++j)
			__mark_free(omap->orders[j],
			            pm_to_pnum(paddr - omap->base), order,
			            omap->order, j);

		return;
	}
}

/**
 * Helper function for marking a page used.
 *
 * @param op Order node pointer.
 * @param pnum Page number to mark used.
 * @param tgt Target order.
 * @param src Source order.
 * @param dst Destination order.
 * @return \ref true if order is filled, \ref false otherwise.
 */
static bool __mark_used(mm_node_t *op, pnum_t pnum, enum mm_order tgt,
                        enum mm_order src, enum mm_order dst)
{
	size_t idx = pnum_to_index(pnum, src);

	if (src == dst) {
		struct mm_leaf *o = (struct mm_leaf *)op;
		bitmap_set(o->used, idx);

		if (idx == max_index(src))
			return true;

		return false;
	}

	struct mm_branch *o = (struct mm_branch *)op;
	if (src == tgt) {
		bitmap_set(o->full, idx);

		if (idx == max_index(src))
			return true;

		return false;
	}

	if (__mark_used(o->next[idx], pnum, tgt, src - 1, dst)) {
		bitmap_set(o->full, idx);

		if (idx == max_index(src))
			return true;
	}

	return false;
}

void mark_used(enum mm_order order, pm_t paddr)
{
	for (size_t i = MM_O0; i <= __mm_max_order; ++i) {
		if (!pmap->omap[i])
			continue;

		struct mm_omap *omap = pmap->omap[i];
		if (paddr < omap->base)
			continue;

		for (size_t j = 0; j <= omap->order; ++j)
			__mark_used(omap->orders[j],
			            pm_to_pnum(paddr - omap->base), order,
			            omap->order, j);

		return;
	}
}

/**
 * Look for next free page.
 *
 * @param op Operand node pointer.
 * @param offset Offset to where to start looking for available pages from.
 * @param src Source order.
 * @param dst Destination order.
 * @return Page number of found index.
 */
static pnum_t __enum_order(mm_node_t *op, pnum_t offset, enum mm_order src,
                           enum mm_order dst)
{
	size_t idx = pnum_to_index(offset, src);

	if (src == dst) {
		struct mm_leaf *o = (struct mm_leaf *)op;
		foreach_not_used_page(o, idx)
		{
			return page << order_offset(src);
		}

		return -1;
	}

	struct mm_branch *o = (struct mm_branch *)op;
	foreach_not_full_page(o, idx)
	{
		/* if the suggested search index is full, the following level
		 * would get an incorrect offset if trying to follow the original
		 * suggestion. */
		if (page != (pnum_t)idx)
			offset = 0;

		pnum_t ret = __enum_order(o->next[page], offset, src - 1, dst);

		if (!(ret < 0))
			return (page << order_offset(src)) + ret;
	}

	return -1;
}

pm_t alloc_page(enum mm_order order, pm_t offset)
{
	if (order > __mm_max_order)
		return 0;

	pnum_t pnum = -1;
	pm_t base = 0;
	struct mm_omap *omap;
	for (size_t i = order; i <= __mm_max_order; ++i) {
		if (!pmap->omap[i])
			continue;

		omap = pmap->omap[i];
		if (offset != 0)
			base = offset - omap->base;

		pnum = __enum_order(omap->orders[order], pm_to_pnum(base),
		                    omap->order, order);

		if (!(pnum < 0))
			break;
	}

	if (pnum < 0)
		return 0;

	pm_t paddr = pnum_to_pm(pnum) + omap->base;
	mark_used(order, paddr);
	return paddr;
}

/**
 * Populate order node map.
 *
 * @param op Address to where to write order node pointer.
 * @param cont Physical address where to continue writing data to.
 * @param src Source order.
 * @param dst Destination order.
 * @param num Number of nodes in this order to populate.
 * @return Physical address to continue from.
 */
static pm_t __populate_order(mm_node_t **op, pm_t cont, enum mm_order src,
                             enum mm_order dst, size_t num)
{
	/* unfortunate that populating the mm info is so complicated */
	if (src == dst) {
		struct mm_leaf *o = (struct mm_leaf *)move_forward(
			cont, sizeof(struct mm_leaf));

		o->entries = num;
		o->used = (mm_info_t *)move_forward(cont, state_elems(num));
		memset(o->used, 0, state_elems(num));

		*op = (mm_node_t *)o;
		return cont;
	}

	struct mm_branch *o = (struct mm_branch *)move_forward(
		cont, sizeof(struct mm_branch));

	o->entries = num;
	o->full = (mm_info_t *)move_forward(cont, state_elems(num));
	cont = align_up(cont, sizeof(void *));
	o->next = (mm_node_t **)move_forward(cont, next_elems(num));
	memset(o->full, 0, state_elems(num));
	memset(o->next, 0, next_elems(num));

	for (size_t i = 0; i < num; ++i) {
		cont = __populate_order(&o->next[i], cont, src - 1, dst,
		                        order_width(src - 1));
	}

	*op = (mm_node_t *)o;
	return cont;
}

/**
 * Probe order node map.
 *
 * @param cont Number of bytes written so far.
 * @param src Source order.
 * @param dst Destination order.
 * @param num Number of nodes in this order to calculate.
 * @return Number of bytes written so far.
 */
static pm_t __probe_order(pm_t cont, enum mm_order src, enum mm_order dst,
                          size_t num)
{
	if (src == dst) {
		cont += sizeof(struct mm_leaf);
		cont += state_elems(num);
		return cont;
	}

	cont += sizeof(struct mm_branch);
	cont += state_elems(num);
	cont = align_up(cont, sizeof(void *));
	cont += next_elems(num);

	for (size_t i = 0; i < num; ++i)
		cont = __probe_order(cont, src - 1, dst, order_width(src - 1));

	return cont;
}

/** Populate order map.
 *
 * @param omap Address where to write order map pointer.
 * @param cont Address where to continue writing map data.
 * @param base Base of order map.
 * @param entries Number of order node entries in order map.
 * @param order Order of this order map.
 * @return Address to continue writing data to.
 */
static pm_t __populate_omap(struct mm_omap **omap, pm_t cont, pm_t base,
                            size_t entries, enum mm_order order)
{
	struct mm_omap *lomap = (struct mm_omap *)move_forward(
		cont, sizeof(struct mm_omap));
	memset(lomap, 0, sizeof(struct mm_omap));

	lomap->orders = (mm_node_t **)move_forward(
		cont, (order + 1) * sizeof(mm_node_t **));
	memset(lomap->orders, 0, (order + 1) * sizeof(mm_node_t **));

	lomap->order = order;
	lomap->base = base;

	for (size_t i = 0; i <= order; ++i)
		cont = __populate_order(&lomap->orders[i], cont, order, i,
		                        entries);

	*omap = lomap;
	return cont;
}

/**
 * Probe order map.
 *
 * @param cont Number of bytes written so far.
 * @param entries Number of order node entries in this order map.
 * @param order Order of this order map.
 * @return Number of bytes written so far.
 */
static pm_t __probe_omap(pm_t cont, size_t entries, enum mm_order order)
{
	cont += sizeof(struct mm_omap);
	cont += (order + 1) * sizeof(mm_node_t **);

	for (size_t i = 0; i <= order; ++i)
		cont = __probe_order(cont, order, i, entries);

	return cont;
}

/* only call from init */
pm_t populate_pmap(pm_t ram_base, size_t ram_size, pm_t cont)
{
	pm_t start = cont;
	pmap = (struct mm_pmap *)move_forward(cont, sizeof(struct mm_pmap));
	memset(pmap, 0, sizeof(struct mm_pmap));

	pm_t ram_region = ram_base;
	size_t ram_left = ram_size;
	for (ssize_t i = __mm_max_order; i >= MM_O0; --i) {
		size_t entries = ram_left / __mm_sizes[i];
		if (entries == 0)
			continue;

		cont = __populate_omap(&pmap->omap[i], cont, ram_region,
		                       entries, i);

		ram_left -= __mm_sizes[i] * entries;
		ram_region += (__mm_sizes[i] * entries);
	}

	return cont - start;
}

/* not a huge fan of having a separate probe_pmap function as that seems like an
 * easy way to cause weird bugs. Should always at least check that probe_pmap
 * returns the same value as populate_pmap, or possibly even add in some method
 * to combine the two? */
pm_t probe_pmap(pm_t ram_base, size_t ram_size)
{
	pm_t cont = 0;

	cont += sizeof(struct mm_pmap);

	pm_t ram_region = ram_base;
	size_t ram_left = ram_size;
	for (ssize_t i = __mm_max_order; i >= MM_O0; --i) {
		size_t entries = ram_left / __mm_sizes[i];
		if (entries == 0)
			continue;

		cont = __probe_omap(cont, entries, i);

		ram_left -= __mm_sizes[i] * entries;
		ram_region += (__mm_sizes[i] * entries);
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

	pm_t ram_size = __get_ramtop(fdt) - RAM_BASE;
	pm_t ram_base = (pm_t)__va(RAM_BASE);

	pm_t initrd_top = get_initrdtop(fdt);
	pm_t fdt_top = __get_fdttop(fdt);

	/* find probably most suitable contiguous region of ram for our physical
	 * ram map */
	pm_t pmap_base = align_up(MAX(initrd_top, fdt_top), sizeof(int));

	size_t probe_size = probe_pmap(ram_base, ram_size);
	size_t actual_size = populate_pmap(ram_base, ram_size, pmap_base);

	if (probe_size != actual_size)
		bug("probe_size (%#lx) != actual_size (%#lx)\n", probe_size,
		    actual_size);

	/* mark init stack, this should be unmapped once we get to executing
	 * processes */
	__mark_area_used((pm_t)__va(PM_STACK_BASE), (pm_t)__va(PM_STACK_TOP));

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
