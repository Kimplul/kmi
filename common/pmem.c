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
 */

#include <apos/mem_nodes.h>
#include <apos/pmem.h>
#include <apos/dmem.h>
#include <apos/debug.h>
#include <apos/initrd.h>
#include <apos/string.h> /* memset */
#include <apos/bits.h> /* is_nset etc */
#include <libfdt.h>

/* NOTE: these are all for pnum_t, i.e. O0_SHIFT is from 0 */
#define __foreach_page(var, start, end, attr, neg)                   \
	for (size_t i = num_indexes(start); i < num_elems(end); ++i) \
	if (var->attr[i] == (mm_info_t)(-1))                         \
	continue;                                                    \
	else                                                         \
	for (pnum_t page = i * MM_OINFO_WIDTH, j = 0;                \
	     j < (pnum_t)MIN((end)-i * MM_OINFO_WIDTH,               \
	                     MM_OINFO_WIDTH);                        \
	     ++j, ++page)                                            \
	if (neg(is_nset(var->attr[i], j)))

#define NEG !
#define foreach_full_page(var, start, order) \
	__foreach_page(var, start, var->entries, full, )

#define foreach_not_full_page(var, start, order) \
	__foreach_page(var, start, var->entries, full, NEG)

#define foreach_used_page(var, start, order) \
	__foreach_page(var, start, var->entries, used, )

#define foreach_not_used_page(var, start, order) \
	__foreach_page(var, start, var->entries, used, NEG)

typedef uint32_t mm_info_t;
typedef void mm_node_t;

struct mm_leaf_t {
	size_t entries;
	mm_info_t *used;
};

struct mm_branch_t {
	size_t entries;
	mm_info_t *full;
	mm_node_t **next;
};

struct mm_omap_t {
	pm_t base;
	mm_node_t **orders;
	enum mm_order order;
};

struct mm_pmap_t {
	struct mm_omap_t *omap[9];
};

static struct mm_pmap_t *pmap = 0;

static void __mark_free(mm_node_t *op, pnum_t pnum, enum mm_order tgt,
                        enum mm_order src, enum mm_order dst)
{
	size_t idx = pnum_to_index(pnum, src);

	if (src == dst) {
		struct mm_leaf_t *o = (struct mm_leaf_t *)op;
		clear_nbit(o->used[__o_container(idx)], __o_bit(idx));
		return;
	}

	struct mm_branch_t *o = (struct mm_branch_t *)op;
	if (src != tgt)
		__mark_free(o->next[idx], pnum, tgt, src - 1, dst);

	/* freeing a page results in always clearing a full bit? */
	clear_nbit(o->full[__o_container(idx)], __o_bit(idx));
}

/* this could probably use an int for status, but eh */
void free_page(enum mm_order order, pm_t paddr)
{
	for (size_t i = MM_O0; i <= __mm_max_order; ++i) {
		if (!pmap->omap[i])
			continue;

		struct mm_omap_t *omap = pmap->omap[i];
		if (paddr < omap->base)
			continue;

		for (size_t j = 0; j < omap->order; ++j)
			__mark_free(omap->orders[j],
			            pm_to_pnum(paddr - omap->base), order,
			            omap->order, j);

		return;
	}
}

static bool __mark_used(mm_node_t *op, pnum_t pnum, enum mm_order tgt,
                        enum mm_order src, enum mm_order dst)
{
	size_t idx = pnum_to_index(pnum, src);

	if (src == dst) {
		struct mm_leaf_t *o = (struct mm_leaf_t *)op;
		set_nbit(o->used[__o_container(idx)], __o_bit(idx));

		if (idx == max_index(src))
			return true;

		return false;
	}

	struct mm_branch_t *o = (struct mm_branch_t *)op;
	if (src == tgt) {
		set_nbit(o->full[__o_container(idx)], __o_bit(idx));

		if (idx == max_index(src))
			return true;

		return false;
	}

	if (__mark_used(o->next[idx], pnum, tgt, src - 1, dst)) {
		set_nbit(o->full[__o_container(idx)], __o_bit(idx));

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

		struct mm_omap_t *omap = pmap->omap[i];
		if (paddr < omap->base)
			continue;

		for (size_t j = 0; j <= omap->order; ++j)
			__mark_used(omap->orders[j],
			            pm_to_pnum(paddr - omap->base), order,
			            omap->order, j);

		return;
	}
}

static pnum_t __enum_order(mm_node_t *op, pnum_t offset, enum mm_order src,
                           enum mm_order dst)
{
	size_t idx = pnum_to_index(offset, src);

	if (src == dst) {
		struct mm_leaf_t *o = (struct mm_leaf_t *)op;
		foreach_not_used_page(o, idx, src)
		{
			return page << __o_offset(src);
		}

		return -1;
	}

	struct mm_branch_t *o = (struct mm_branch_t *)op;
	foreach_not_full_page(o, idx, src)
	{
		/* if the suggested search index is full, the following level
		 * would get an incorrect offset if trying to follow the original
		 * suggestion. */
		if (page != (pnum_t)idx)
			offset = 0;

		pnum_t ret = __enum_order(o->next[page], offset, src - 1, dst);

		if (!(ret < 0))
			return (page << __o_offset(src)) + ret;
	}

	return -1;
}

pm_t alloc_page(enum mm_order order, pm_t offset)
{
	if (order > __mm_max_order)
		return 0;

	pnum_t pnum = -1;
	pm_t base = 0;
	struct mm_omap_t *omap;
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

	pm_t paddr = pnum_to_paddr(pnum) + omap->base;
	mark_used(order, paddr);
	return paddr;
}

/* unfortunate that populating the mm info is so complicated */
static pm_t __populate_order(mm_node_t **op, pm_t cont, enum mm_order src,
                             enum mm_order dst, size_t num)
{
	if (src == dst) {
		struct mm_leaf_t *o = (struct mm_leaf_t *)move_forward(
			cont, sizeof(struct mm_leaf_t));

		o->entries = num;
		o->used = (mm_info_t *)move_forward(cont, state_elems(num));
		memset(o->used, 0, state_elems(num));

		*op = (mm_node_t *)o;
		return cont;
	}

	struct mm_branch_t *o = (struct mm_branch_t *)move_forward(
		cont, sizeof(struct mm_branch_t));

	o->entries = num;
	o->full = (mm_info_t *)move_forward(cont, state_elems(num));
	cont = align_up(cont, sizeof(void *));
	o->next = (mm_node_t **)move_forward(cont, next_elems(num));
	memset(o->full, 0, state_elems(num));
	memset(o->next, 0, next_elems(num));

	for (size_t i = 0; i < num; ++i) {
		cont = __populate_order(&o->next[i], cont, src - 1, dst,
		                        __o_width(src - 1));
	}

	*op = (mm_node_t *)o;
	return cont;
}

static pm_t __probe_order(pm_t cont, enum mm_order src, enum mm_order dst,
                          size_t num)
{
	if (src == dst) {
		cont += sizeof(struct mm_leaf_t);
		cont += state_elems(num);
		return cont;
	}

	cont += sizeof(struct mm_branch_t);
	cont += state_elems(num);
	cont = align_up(cont, sizeof(void *));
	cont += next_elems(num);

	for (size_t i = 0; i < num; ++i)
		cont = __probe_order(cont, src - 1, dst, __o_width(src - 1));

	return cont;
}

static pm_t __populate_omap(struct mm_omap_t **omap, pm_t cont, pm_t base,
                            size_t entries, enum mm_order order)
{
	struct mm_omap_t *lomap = (struct mm_omap_t *)move_forward(
		cont, sizeof(struct mm_omap_t));
	memset(lomap, 0, sizeof(struct mm_omap_t));

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

static pm_t __probe_omap(pm_t cont, size_t entries, enum mm_order order)
{
	cont += sizeof(struct mm_omap_t);
	cont += (order + 1) * sizeof(mm_node_t **);

	for (size_t i = 0; i <= order; ++i)
		cont = __probe_order(cont, order, i, entries);

	return cont;
}

/* only call from init */
pm_t populate_pmap(pm_t ram_base, size_t ram_size, pm_t cont)
{
	pm_t start = cont;
	pmap = (struct mm_pmap_t *)move_forward(cont, sizeof(struct mm_pmap_t));
	memset(pmap, 0, sizeof(struct mm_pmap_t));

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

	cont += sizeof(struct mm_pmap_t);

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

static pm_t __get_fdttop(void *fdt)
{
	const char *b = (const char *)fdt;
	return (pm_t)(b + fdt_totalsize(fdt));
}

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

	init_mem_blocks();

	init_devmem((pm_t)__pa(ram_base), (pm_t)__pa(ram_base + ram_size));
}
