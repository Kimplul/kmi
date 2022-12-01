/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file mem_regions.c
 * Memory region handling, used by both device memory and user virtual memory
 * subsystems.
 */

#include <apos/mem_regions.h>
#include <apos/mem_nodes.h>
#include <apos/pmem.h>
#include <apos/bits.h>
#include <apos/mem.h>

/**
 * Readability wrapper for marking region used.
 *
 * @param r Region flags to set.
 */
#define mark_region_used(r) set_bit(r, MR_USED)

/**
 * Readability wrapper for marking region unused.
 *
 * @param r Region flags to clear.
 */
#define mark_region_unused(r) clear_bit(r, MR_USED)

/* pretty major slowdown when we get to some really massive numbers, not
 * entirely sure why. Will need to check up on this at some point, have I
 * somehow managed to come up with a _very_ bad situation for my sp_trees?
 *
 * EDIT: apparently, yeah. Max depth of 106 with a million entries, interesting.
 * I guess since in this scenario all sizes are 1, and I just shove everything
 * to the right? Maybe?
 *
 * EDIT upon EDIT: yeah, when taking the start position of the region into
 * account we get a much more sensible max depth of 39 for 5 million entries.
 * Seems I have found a weakness in sp_trees :D
 *
 * Duplicate entries don't work well with any trees, I think. Good to know,
 * maybe not even anything with sp_trees but more a weakness of binary trees in
 * general?
 */

/**
 * Insert free memory region.
 *
 * @param r Memory region root to insert \c m into.
 * @param m Free memory region to insert.
 * @return \c m.
 */
static struct mem_region *__insert_free_region(struct mem_region_root *r,
                                               struct mem_region *m)
{
	struct sp_node *n = sp_root(&r->free_regions), *p = NULL;
	vm_t start = m->start;
	size_t size = m->end - m->start;
	enum sp_dir d = LEFT;

	m->sp_n = (struct sp_node){ 0 };

	while (n) {
		struct mem_region *t = mem_container(n);
		size_t nsize = t->end - t->start;
		p = n;

		if (size < nsize) {
			n = sp_left(n);
			d = LEFT;
		}

		else if (size > nsize) {
			n = sp_right(n);
			d = RIGHT;
		}

		else if (start < t->start) {
			n = sp_left(n);
			d = LEFT;
		}

		else {
			n = sp_right(n);
			d = RIGHT;
		}
	}

	if (sp_root(&r->free_regions))
		sp_insert(&sp_root(&r->free_regions), p, &m->sp_n, d);
	else
		sp_root(&r->free_regions) = &m->sp_n;

	return m;
}

/**
 * Insert used memory region.
 *
 * @param r Memory region root to insert \c m into.
 * @param m Memory region to insert.
 * @return \c m.
 */
static struct mem_region *__insert_used_region(struct mem_region_root *r,
                                               struct mem_region *m)
{
	struct sp_node *n = sp_root(&r->used_regions), *p = NULL;
	vm_t start = m->start;
	enum sp_dir d = LEFT;

	m->sp_n = (struct sp_node){ 0 };

	while (n) {
		struct mem_region *t = mem_container(n);

		p = n;

		if (start < t->start) {
			n = sp_left(n);
			d = LEFT;
		}

		else {
			/* we should never encounter a situation where start =
			 * t->start */
			n = sp_right(n);
			d = RIGHT;
		}
	}

	if (sp_root(&r->used_regions))
		sp_insert(&sp_root(&r->used_regions), p, &m->sp_n, d);
	else
		sp_root(&r->used_regions) = &m->sp_n;

	return m;
}

stat_t init_region(struct mem_region_root *r, vm_t start, size_t arena_size)
{
	/* convert bytes to pages */
	start = __page(start);
	arena_size = __page(arena_size);
	struct mem_region *m = get_mem_node();
	m->start = start;
	m->end = start + arena_size;
	__insert_free_region(r, m);

	return OK;
}

/**
 * Destroy memory region and all its children.
 *
 * @param n \ref sp_node of memory region to destroy.
 */
static void __destroy_region(struct sp_node *n)
{
	if (!n)
		return;

	if (sp_left(n))
		__destroy_region(sp_left(n));

	if (sp_right(n))
		__destroy_region(sp_right(n));

	struct mem_region *m = mem_container(n);
	free_mem_node(m);
}

stat_t destroy_region(struct mem_region_root *r)
{
	__destroy_region(sp_root(&r->free_regions));
	__destroy_region(sp_root(&r->used_regions));
	/** \todo error checking? */
	return OK;
}

/* interestingly this is now the main bottleneck :D
 *
 * eh, it's not a massive thing I guess, maybe the code could be a bit quicker
 * but I mean 10 000 000 memory allocations in 20 s is good enough for now
 * */
struct mem_region *find_used_region(struct mem_region_root *r, vm_t start)
{
	/** @todo check that start is aligned to page boundary? */
	vm_t ref = __page(start);
	struct sp_node *n = sp_root(&r->used_regions);
	while (n) {
		struct mem_region *t = mem_container(n);
		if (ref == t->start)
			return t;

		if (ref < t->start)
			n = sp_left(n);
		else
			n = sp_right(n);
	}

	return 0;
}

/**
 * Create memory region.
 *
 * @param start Start of region.
 * @param end End of region.
 * @param prev Previous region.
 * @param next Next region.
 * @return Created region.
 */
static struct mem_region *__create_region(vm_t start, vm_t end,
                                          struct mem_region *prev,
                                          struct mem_region *next)
{
	struct mem_region *m = get_mem_node();
	m->start = start;
	m->end = end;
	m->prev = prev;
	m->next = next;
	return m;
}

/**
 * Get first order size smaller than \c s in bytes.
 *
 * @param s Size to look for.
 * @return Size of first order smaller than \c s.
 */
static size_t po_align(size_t s)
{
	for (size_t o = __mm_max_order; o > 0; --o) {
		if (s >= order_size(o))
			return order_size(o);
	}

	return 0;
}

struct mem_region *find_closest_used_region(struct mem_region_root *r,
                                            vm_t start)
{
	struct mem_region *closest = 0;
	size_t md = (size_t)(-1);
	struct sp_node *n = sp_root(&r->used_regions);
	if (!n)
		return mem_container(sp_root(&r->free_regions));

	while (n) {
		struct mem_region *t = mem_container(n);
		size_t d = ABS((ssize_t)start - (ssize_t)t->start);

		if (d == 0) /* exact match */
			return t;

		if (d < md) { /* closest so far */
			closest = t;
			md = d;
		}

		if (start < t->start)
			n = sp_left(n);
		else
			n = sp_right(n);
	}

	return closest;
}

/* should probably document this a bit better but in short, look for the "best"
 * free block, meaning one that is hopefully aligned so as to allow us to later
 * map it to higher order pages. If no block is found such that that is
 * possible, also keep track of the smallest block that we found that the region
 * still fits in, unaligned. If none of these criteria are met, a NULL is
 * returned. Note that this does not check *all* possible memory blocks, only
 * going up in increasing size so as to save time. */
struct mem_region *find_free_region(struct mem_region_root *r, size_t size,
                                    size_t *align)
{
	*align = 0;
	size_t offset = __page(po_align(__addr(size)));
	struct mem_region *quick_best = 0;
	struct sp_node *n = sp_root(&r->free_regions);
	while (n) {
		struct mem_region *t = mem_container(n);
		vm_t start = align_up(t->start, offset);

		size_t qsize = t->end - t->start;
		size_t bsize = t->end - start;

		if (!quick_best && size <= qsize)
			quick_best = t;

		if (size <= bsize) {
			*align = start - t->start;
			return t;
		}

		n = sp_right(n);
	}

	return quick_best;
}

struct mem_region *find_first_region(struct mem_region_root *r)
{
	/* get used region with smallest address, likely also close to the start
	 * of the linked list */
	struct mem_region *m = find_closest_used_region(r, 0);
	while (m->prev) {
		m = m->prev;
	}

	return m;
}

/**
 * Carve out new used memory region from free memory region.
 *
 * @param r Memory region root to work in.
 * @param m Free memory region to carve used memory region out of.
 * @param pages Number of base order pages to give used region.
 * @param align Alignment of used region. In this case, start of used region
 * from start of free region.
 * @param flags Flags of used region.
 * @return Start address of used region.
 */
static vm_t __partition_region(struct mem_region_root *r, struct mem_region *m,
                               size_t pages, size_t align, vmflags_t flags)
{
	sp_remove(&sp_root(&r->free_regions), &m->sp_n);

	vm_t pre_start = m->start;
	vm_t pre_end = pre_start + align;

	vm_t start = pre_end;
	vm_t end = start + pages;

	vm_t post_start = end;
	vm_t post_end = m->end;

	if (pre_start != pre_end) {
		struct mem_region *n =
			__create_region(pre_start, pre_end, m->prev, m);
		m->prev = n;
		if (n->prev)
			n->prev->next = n;

		__insert_free_region(r, n);
	}

	if (post_start != post_end) {
		struct mem_region *n =
			__create_region(post_start, post_end, m, m->next);
		m->next = n;
		if (n->next)
			n->next->prev = n;

		__insert_free_region(r, n);
	}

	m->end = end;
	m->start = start;
	m->flags = flags;
	mark_region_used(m->flags);
	__insert_used_region(r, m);
	return __addr(start);
}

/* apparently Linux doesn't necessarily give a shit about mmap hints, so I'll
 * just ignore them for now. Note that alloc_region should only be used when
 * mmap is called with MAP_ANON, all other situations should be handled in some
 * fs server */
vm_t alloc_region(struct mem_region_root *r, size_t size, size_t *actual_size,
                  vmflags_t flags)
{
	size_t asize = align_up(size, BASE_PAGE_SIZE);
	if (actual_size)
		*actual_size = asize;

	size_t pages = __page(asize);

	/* find best fitting, alignment etc. */
	size_t align = 0;
	struct mem_region *m = find_free_region(r, pages, &align);
	if (!m)
		return 0;

	return __partition_region(r, m, pages, align, flags);
}

vm_t alloc_fixed_region(struct mem_region_root *r, vm_t start, size_t size,
                        size_t *actual_size, vmflags_t flags)
{
	size_t asize = align_up(size, BASE_PAGE_SIZE);
	if (actual_size)
		*actual_size = asize;

	size_t pages = __page(asize);
	start = __page(start);

	struct mem_region *m = find_closest_used_region(r, start);
	if (!m)
		return 0;

	/* locate actual region where start is between the region start and end */
	while (!((m->start <= start) && (start <= m->end))) {
		if (start > m->start)
			m = m->next;
		else
			m = m->prev;
	}

	/* if region is already in use, forget it */
	if (is_region_used(m))
		return 0;

	/* region is too small */
	if (start + pages > m->end)
		return 0;

	/* actually start marking region used */
	return __partition_region(r, m, pages, start - m->start, flags);
}

/**
 * Try to coalesce two adjacent memory regions, iterating left.
 *
 * @param r Memory region root to work in.
 * @param m Memory region to start trying to coalesce.
 */
static void __try_coalesce_prev(struct mem_region_root *r, struct mem_region *m)
{
	while (m) {
		if (!m || is_region_used(m))
			return;

		struct mem_region *p = m->prev;
		if (!p || is_region_used(p))
			return;

		m->start = p->start;
		m->prev = p->prev;

		if (m->prev)
			m->prev->next = m;

		sp_remove(&sp_root(&r->free_regions), &p->sp_n);
		free_mem_node(p);

		m = m->prev;
	}
}

/**
 * Try to coalesce two adjacent memory region, iterating right.
 *
 * @param r Memory region root to work in.
 * @param m Memory region to start trying to coalesce.
 */
static void __try_coalesce_next(struct mem_region_root *r, struct mem_region *m)
{
	while (m) {
		if (!m || is_region_used(m))
			return;

		struct mem_region *n = m->next;
		if (!n || is_region_used(n))
			return;

		m->end = n->end;
		m->next = n->next;

		if (m->next)
			m->next->prev = m;

		sp_remove(&sp_root(&r->free_regions), &n->sp_n);
		free_mem_node(n);

		m = m->next;
	}
}

/**
 * Try coalescing memory regions.
 *
 * @param r Memory region root to work in.
 * @param m Memory region to start trying to coalesce.
 */
static void __try_coalesce_regions(struct mem_region_root *r,
                                   struct mem_region *m)
{
	__try_coalesce_prev(r, m);
	__try_coalesce_next(r, m);
}

stat_t free_region(struct mem_region_root *r, vm_t start)
{
	/* addr not aligned to page boundary, corrupted or incorrect pointer */
	if (!is_aligned(start, BASE_PAGE_SIZE))
		return ERR_ALIGN;

	struct mem_region *m = find_used_region(r, start);
	if (!m)
		return ERR_NF;

	return free_known_region(r, m);
}

stat_t free_known_region(struct mem_region_root *r, struct mem_region *m)
{
	sp_remove(&sp_root(&r->used_regions), &m->sp_n);
	mark_region_unused(m->flags);

	__try_coalesce_regions(r, m);
	__insert_free_region(r, m);
	return OK;
}

/* assuming start is chosen to start on an aligned border, this should choose
 * the 'optimal' fit for the mapping.
 *
 * NOTE: not actually optimal, this doesn't bother to go through possible
 * permutations etc. which would be slow and I don't want to implement it.
 */
vm_t map_fill_region(struct vmem *b, region_callback_t *mem_handler,
                     pm_t offset, vm_t start, size_t bytes, vmflags_t flags,
                     void *data)
{
	pm_t runner = __page(start);
	size_t pages = __pages(bytes);
	enum mm_order top = __mm_max_order;

	/* actual start might not be the same as the user specified start */
	start = __addr(runner);

	for (; pages; top--) {
		size_t o_size = order_size(top);
		size_t o_pages = __pages(o_size);

		/* NULL does pass this check, so technically all NULL pages are
		 * aligned, but they're caught in the while expr so this should
		 * work even if someone tries to map NULL */
		if (!is_aligned(runner, o_pages))
			continue;

		while (pages >= o_pages) {
			stat_t res = mem_handler(b, &offset, __addr(runner),
			                         flags, top, data);
			if (res > 0)
				break;

			if (res < 0)
				return 0;

			pages -= o_pages;
			runner += o_pages;
		}
	}

	return start;
}
