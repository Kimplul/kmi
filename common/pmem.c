#include <apos/pmem.h>
#include <apos/string.h> /* memset */
#include <apos/bits.h> /* __is_nset etc */

#if defined(O0_WIDTH)
#define MM_O0_SHIFT (O0_WIDTH)
#define MM_O0_WIDTH (1UL << (O0_WIDTH))
#define MM_O0_SIZE  (1UL << MM_O0_SHIFT << PAGE_SHIFT)
#else
#define MM_O0_SHIFT 0
#define MM_O0_WIDTH 0
#define MM_O0_SIZE  0
#endif

#if defined(O1_WIDTH)
#define MM_O1_SHIFT (MM_O0_SHIFT + (O1_WIDTH))
#define MM_O1_WIDTH (1UL << (O1_WIDTH))
#define MM_O1_SIZE  (1UL << MM_O1_SHIFT << PAGE_SHIFT)
#else
#define MM_O1_SHIFT 0
#define MM_O1_SHIFT 0
#define MM_O1_SIZE  0
#endif

#if defined(O2_WIDTH)
#define MM_O2_SHIFT (MM_O1_SHIFT + (O2_WIDTH))
#define MM_O2_WIDTH (1UL << (O2_WIDTH))
#define MM_O2_SIZE  (1UL << MM_O2_SHIFT << PAGE_SHIFT)
#else
#define MM_O2_SHIFT 0
#define MM_O2_WIDTH 0
#define MM_O2_SIZE  0
#endif

#if defined(O3_WIDTH)
#define MM_O3_SHIFT (MM_O2_SHIFT + (O3_WIDTH))
#define MM_O3_WIDTH (1UL << (O3_WIDTH))
#define MM_O3_SIZE  (1UL << MM_O3_SHIFT << PAGE_SHIFT)
#else
#define MM_O3_SHIFT 0
#define MM_O3_WIDTH 0
#define MM_O3_SIZE  0
#endif

#if defined(O4_WIDTH)
#define MM_O4_SHIFT (MM_O3_SHIFT + (O4_WIDTH))
#define MM_O4_SHIFT (1UL << (O4_WIDTH))
#define MM_O4_SIZE  (1UL << MM_O4_SHIFT << PAGE_SHIFT)
#else
#define MM_O4_SHIFT 0
#define MM_O4_WIDTH 0
#define MM_O4_SIZE  0
#endif

#if defined(O5_WIDTH)
#define MM_O5_SHIFT (MM_O4_SHIFT + (O5_WIDTH))
#define MM_O5_WIDTH (1UL << (O5_WIDTH))
#define MM_O5_SIZE  (1UL << MM_O5_SHIFT << PAGE_SHIFT)
#else
#define MM_O5_SHIFT 0
#define MM_O5_WIDTH 0
#define MM_O5_SIZE  0
#endif

#if defined(O6_WIDTH)
#define MM_O6_SHIFT (MM_O5_SHIFT + (O6_WIDTH))
#define MM_O6_WIDTH (1UL << (O6_WIDTH))
#define MM_O6_SIZE  (1UL << MM_O6_SHIFT << PAGE_SHIFT)
#else
#define MM_O6_SHIFT 0
#define MM_O6_WIDTH 0
#define MM_O6_SIZE  0
#endif

#if defined(O7_WIDTH)
#define MM_O7_SHIFT (MM_O6_SHIFT + (O7_WIDTH))
#define MM_O7_WIDTH (1UL << (O7_WIDTH))
#define MM_O7_SIZE  (1UL << MM_O7_SHIFT << PAGE_SHIFT)
#else
#define MM_O7_SHIFT 0
#define MM_O7_WIDTH 0
#define MM_O7_SIZE  0
#endif

#if defined(O8_WIDTH)
#define MM_O8_SHIFT (MM_O7_SHIFT + (O8_WIDTH))
#define MM_O8_WIDTH (1UL << (O8_WIDTH))
#define MM_O8_SIZE  (1UL << MM_O8_SHIFT << PAGE_SHIFT)
#else
#define MM_O8_SHIFT 0
#define MM_O8_WIDTH 0
#define MM_O8_SIZE  0
#endif

#if defined(O9_WIDTH)
#define MM_O9_SHIFT (MM_O8_SHIFT + (O9_WIDTH))
#define MM_O9_WIDTH (1UL << (O9_WIDTH))
#define MM_O9_SIZE  (1UL << MM_O9_SHIFT << PAGE_SHIFT)
#else
#define MM_O9_SHIFT 0
#define MM_O9_WIDTH 0
#define MM_O9_SIZE  0
#endif

#define MM_OINFO_WIDTH (sizeof(mm_info_t) * 8)

#define pnum_to_index(pnum, order) (((pnum) >> __o_offset(order)) & (__o_width(order) - 1))
#define paddr_to_pnum(paddr) ((paddr) >> PAGE_SHIFT)
#define pnum_to_paddr(pnum) ((pnum) << PAGE_SHIFT)

#define move_forward(var, num) (((var) += (num)) - (num))

#define move_paddr(paddr, base, offset) ((((paddr_t)(paddr)) - (base)) + (offset))
#define num_elems(num) (((num) + MM_OINFO_WIDTH - 1) / MM_OINFO_WIDTH)
#define num_indexes(num) ((num) / MM_OINFO_WIDTH)
#define index_elems(num) ((num) / MM_OINFO_WIDTH)
#define state_elems(num) (sizeof(mm_info_t) * (num_elems(num)))
#define next_elems(num) (sizeof(void *) * (num))
#define max_index(order) (__o_width(order) - 1)

#define __o_offset(order) (mm_shifts[order])
#define __o_width(order) (mm_widths[order])
#define __o_elems(order) (mm_widths[order] / MM_OINFO_WIDTH)

#define __o_container(idx) ((idx) / MM_OINFO_WIDTH)
#define __o_bit(idx) ((idx) & (MM_OINFO_WIDTH - 1))

#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define __foreach_page(var, start, end, attr, neg)\
	for(size_t i = num_indexes(start); i < num_elems(end); ++i)\
	if(var->attr[i] == (mm_info_t)(-1)) continue;\
	else for(pnum_t page = i * MM_OINFO_WIDTH, j = 0;\
			j < (pnum_t)MIN((end) - i * MM_OINFO_WIDTH, MM_OINFO_WIDTH);\
			++j, ++page)\
		if(neg(__is_nset(var->attr[i], j)))

#define NEG !
#define foreach_full_page(var, start, order)\
	__foreach_page(var, start, var->entries, full, )

#define foreach_not_full_page(var, start, order)\
	__foreach_page(var, start, var->entries, full, NEG)

#define foreach_used_page(var, start, order)\
	__foreach_page(var, start, var->entries, used, )

#define foreach_not_used_page(var, start, order)\
	__foreach_page(var, start, var->entries, used, NEG)

static const size_t mm_shifts[] = {
	MM_O0_SHIFT,
	MM_O1_SHIFT,
	MM_O2_SHIFT,
	MM_O3_SHIFT,
	MM_O4_SHIFT,
	MM_O6_SHIFT,
	MM_O7_SHIFT,
	MM_O8_SHIFT,
	MM_O9_SHIFT,
};

static const size_t mm_widths[] = {
	MM_O0_WIDTH,
	MM_O1_WIDTH,
	MM_O2_WIDTH,
	MM_O3_WIDTH,
	MM_O4_WIDTH,
	MM_O5_WIDTH,
	MM_O6_WIDTH,
	MM_O7_WIDTH,
	MM_O8_WIDTH,
	MM_O9_WIDTH,
};

static const size_t mm_sizes[] = {
	MM_O0_SIZE,
	MM_O1_SIZE,
	MM_O2_SIZE,
	MM_O3_SIZE,
	MM_O4_SIZE,
	MM_O5_SIZE,
	MM_O6_SIZE,
	MM_O7_SIZE,
	MM_O8_SIZE,
	MM_O9_SIZE,
};

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
	paddr_t base;
	mm_node_t **orders;
	enum mm_order_t order;
};

struct mm_pmap_t {
	struct mm_omap_t *omap[9];
};

static struct mm_pmap_t *pmap = 0;

static void __mark_free(mm_node_t * op, pnum_t pnum,
	enum mm_order_t src, enum mm_order_t dst)
{
	size_t idx = pnum_to_index(pnum, src);

	if (src == dst) {
		struct mm_leaf_t *o = (struct mm_leaf_t *)op;
		__clear_nbit(o->used[__o_container(idx)], __o_bit(idx));
		return;
	}

	struct mm_branch_t *o = (struct mm_branch_t *)op;
	__mark_free(o->next[idx], pnum, src - 1, dst);
	/* freeing a page results in always clearing a full bit? */
	__clear_nbit(o->full[__o_container(idx)], __o_bit(idx));
}

void free_page(enum mm_order_t order, paddr_t paddr)
{
	for (ssize_t i = MAX_ORDER; i >= order; --i) {
		if (!pmap->omap[i])
			continue;

		struct mm_omap_t *omap = pmap->omap[i];
		if (paddr < omap->base)
			continue;

		for (size_t j = 0; j < omap->order; ++j)
			__mark_free(omap->orders[j],
				paddr_to_pnum(paddr - omap->base),
				omap->order, j);

	}
}

static bool __mark_used(mm_node_t * op, pnum_t pnum, enum mm_order_t tgt,
	enum mm_order_t src, enum mm_order_t dst)
{
	size_t idx = pnum_to_index(pnum, src);

	if (src == dst) {
		struct mm_leaf_t *o = (struct mm_leaf_t *)op;
		__set_nbit(o->used[__o_container(idx)], __o_bit(idx));

		if (idx == max_index(src))
			return true;

		return false;
	}

	struct mm_branch_t *o = (struct mm_branch_t *)op;
	if (src == tgt) {
		__set_nbit(o->full[__o_container(idx)], __o_bit(idx));

		if (idx == max_index(src))
			return true;

		return false;
	}

	if (__mark_used(o->next[idx], pnum, tgt, src - 1, dst)) {
		__set_nbit(o->full[__o_container(idx)], __o_bit(idx));

		if (idx == max_index(src))
			return true;
	}

	return false;
}

void mark_used(enum mm_order_t order, paddr_t paddr)
{
	for (ssize_t i = MAX_ORDER; i >= MM_O0; --i) {
		if (!pmap->omap[i])
			continue;

		struct mm_omap_t *omap = pmap->omap[i];
		if (paddr < omap->base)
			continue;

		for (size_t j = 0; j <= omap->order; ++j)
			__mark_used(omap->orders[j],
				paddr_to_pnum(paddr - omap->base),
				order, omap->order, j);

		return;
	}
}

static pnum_t __enum_order(mm_node_t * op, pnum_t offset,
	enum mm_order_t src, enum mm_order_t dst)
{
	size_t idx = pnum_to_index(offset, src);

	if (src == dst) {
		struct mm_leaf_t *o = (struct mm_leaf_t *)op;
		foreach_not_used_page(o, idx, src) {
			return page << __o_offset(src);
		}

		return -1;
	}

	struct mm_branch_t *o = (struct mm_branch_t *)op;
	foreach_not_full_page(o, idx, src) {
		/* if the suggested search index is full, the following level
		 * would get an incorrect offset if trying to follow the original
		 * suggestion. */
		if (page != (pnum_t) idx)
			offset = 0;

		pnum_t ret = __enum_order(o->next[page], offset, src - 1, dst);

		if (!(ret < 0))
			return (page << __o_offset(src)) + ret;
	}

	return -1;
}

paddr_t alloc_page(enum mm_order_t order, paddr_t offset)
{
	if (order > MAX_ORDER)
		return 0;

	pnum_t pnum = -1;
	paddr_t base = 0;
	struct mm_omap_t *omap;
	for (size_t i = order; i <= MAX_ORDER; ++i) {
		if (!pmap->omap[i])
			continue;

		omap = pmap->omap[i];
		if (offset != 0)
			base = offset - omap->base;

		pnum = __enum_order(omap->orders[order],
			paddr_to_pnum(base), omap->order, order);

		if (!(pnum < 0))
			break;
	}

	if (pnum < 0)
		return 0;

	paddr_t paddr = pnum_to_paddr(pnum) + omap->base;
	mark_used(order, paddr);
	return paddr;
}



static void __update_order(mm_node_t * op, paddr_t base, paddr_t offset,
	enum mm_order_t src, enum mm_order_t dst)
{
	if (src == dst) {
		struct mm_leaf_t *o = (struct mm_leaf_t *)op;
		o->used = (mm_info_t *) move_paddr(o->used, base, offset);
		return;
	}

	struct mm_branch_t *o = (struct mm_branch_t *)op;
	o->full = (mm_info_t *) move_paddr(o->full, base, offset);
	for (size_t i = 0; i < o->entries; ++i) {
		__update_order(o->next[i], base, offset, src - 1, dst);
		o->next[i] =
			(mm_node_t **) move_paddr(o->next[i], base, offset);
	}

	o->next = (mm_node_t **) move_paddr(o->next, base, offset);
}

static void __update_omap(struct mm_omap_t *omap, paddr_t base, paddr_t offset)
{
	for (size_t i = MM_O0; i <= omap->order; ++i) {
		__update_order(omap->orders[i], base, offset, omap->order, i);
		omap->orders[i] =
			(mm_node_t *) move_paddr(omap->orders[i], base, offset);
	}

	omap->orders = (mm_node_t **) move_paddr(omap->orders, base, offset);
}

void update_pmap(paddr_t offset)
{
	paddr_t base = (paddr_t) pmap;
	for (size_t i = 0; i <= MAX_ORDER; ++i) {
		if (!pmap->omap[i])
			continue;

		__update_omap(pmap->omap[i], base, offset);
		pmap->omap[i] = (struct mm_omap_t *)move_paddr(pmap->omap[i],
			base, offset);
	}

	pmap = (struct mm_pmap_t *)move_paddr(pmap, base, offset);
}

/* unfortunate that populating the mm info is so complicated */
static paddr_t __populate_order(mm_node_t ** op, paddr_t cont,
	enum mm_order_t src, enum mm_order_t dst, size_t num)
{
	if (src == dst) {
		struct mm_leaf_t *o = (struct mm_leaf_t *)
			move_forward(cont, sizeof(struct mm_leaf_t));

		o->entries = num;
		o->used = (mm_info_t *) move_forward(cont, state_elems(num));
		memset(o->used, 0, state_elems(num));

		*op = (mm_node_t *) o;
		return cont;
	}

	struct mm_branch_t *o = (struct mm_branch_t *)
		move_forward(cont, sizeof(struct mm_branch_t));

	o->entries = num;
	o->full = (mm_info_t *) move_forward(cont, state_elems(num));
	o->next = (mm_node_t **) move_forward(cont, next_elems(num));
	memset(o->full, 0, state_elems(num));
	memset(o->next, 0, next_elems(num));

	for (size_t i = 0; i < num; ++i) {
		cont = __populate_order(&o->next[i], cont,
			src - 1, dst, __o_width(src - 1));
	}

	*op = (mm_node_t *) o;
	return cont;
}

static paddr_t __populate_omap(struct mm_omap_t **omap, paddr_t cont,
	paddr_t base, size_t entries, enum mm_order_t order)
{
	struct mm_omap_t *lomap = (struct mm_omap_t *)
		move_forward(cont, sizeof(struct mm_omap_t));

	memset(lomap, 0, sizeof(struct mm_omap_t));

	lomap->orders = (mm_node_t **) move_forward(cont,
		(order + 1) * sizeof(mm_node_t **));
	memset(lomap->orders, 0, (order + 1) * sizeof(mm_node_t **));

	lomap->order = order;
	lomap->base = base;

	for (size_t i = 0; i <= order; ++i)
		cont = __populate_order(&lomap->orders[i], cont,
			order, i, entries);

	*omap = lomap;
	return cont;
}
/* only call from init */
void populate_pmap(paddr_t ram_base, size_t ram_size, paddr_t cont)
{
	pmap = (struct mm_pmap_t *)move_forward(cont, sizeof(struct mm_pmap_t));
	memset(pmap, 0, sizeof(struct mm_pmap_t));

	paddr_t ram_region = ram_base;
	size_t ram_left = ram_size;
	for (ssize_t i = MAX_ORDER; i >= MM_O0; --i) {
		size_t entries = ram_left / mm_sizes[i];
		if (entries == 0)
			continue;

		cont = __populate_omap(&pmap->omap[i], cont,
			ram_region, entries, i);

		ram_left -= mm_sizes[i] * entries;
		ram_region += (mm_sizes[i] * entries);
	}
}

/* only call from kernel */
void init_pmap(void *p)
{
	pmap = (struct mm_pmap_t *)p;
}
