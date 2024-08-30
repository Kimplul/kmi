/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file regions.c
 * Memory region handling, used by both device memory and user virtual memory
 * subsystems.
 */

#include <kmi/syscalls.h>
#include <kmi/regions.h>
#include <kmi/assert.h>
#include <kmi/pmem.h>
#include <kmi/bits.h>
#include <kmi/mem.h>

/** Memory node "subsystem" instance. */
static struct node_root root;

void init_mem_nodes()
{
	init_nodes(&root, sizeof(struct mem_region));
}

void destroy_mem_nodes()
{
	destroy_nodes(&root);
}

/**
 * Allocate a new memory region node and return it.
 *
 * @return New memory region node.
 */
static struct mem_region *get_mem_node()
{
	struct mem_region *r = get_node(&root);
	memset(r, 0, sizeof(struct mem_region));
	return r;
}

/**
 * Free a memory region node.
 *
 * @param m Memory region node to free.
 */
static void free_mem_node(struct mem_region *m)
{
	free_node(&root, (void *)m);
}

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
static __inline struct mem_region *__insert_free_region(
	struct mem_region_root *r,
	struct mem_region *m)
{
	/* this could be simplified by using my gsptrees in kmx, but at least
	 * this ensures 'inlining' of the condition checking so I'll let it stay
	 * for now */
	struct sp_node *n = sp_root(&r->free_regions), *p = NULL;
	vm_t start = m->start;
	size_t size = m->end - m->start;
	enum sp_dir d = SP_LEFT;

	m->sp_n = (struct sp_node){ 0 };

	while (n) {
		struct mem_region *t = mem_container(n);
		size_t nsize = t->end - t->start;
		p = n;

		if (size < nsize) {
			n = sp_left(n);
			d = SP_LEFT;
		}

		else if (size > nsize) {
			n = sp_right(n);
			d = SP_RIGHT;
		}

		else if (start < t->start) {
			/* note that blocks with smaller addresses go to the
			 * left */
			n = sp_left(n);
			d = SP_LEFT;
		}

		else {
			n = sp_right(n);
			d = SP_RIGHT;
		}
	}

	sp_insert(&sp_root(&r->free_regions), p, &m->sp_n, d);
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
	enum sp_dir d = SP_LEFT;

	m->sp_n = (struct sp_node){ 0 };

	while (n) {
		struct mem_region *t = mem_container(n);

		p = n;

		if (start < t->start) {
			n = sp_left(n);
			d = SP_LEFT;
		}

		else {
			/* we should never encounter a situation where start =
			 * t->start */
			n = sp_right(n);
			d = SP_RIGHT;
		}
	}

	sp_insert(&sp_root(&r->used_regions), p, &m->sp_n, d);
	return m;
}

stat_t init_region(struct mem_region_root *r, vm_t start, size_t arena_size,
                   size_t reserved)
{
	/* convert bytes to pages */
	start = __page(start);
	arena_size = __page(arena_size);
	struct mem_region *m = get_mem_node();
	m->start = start;
	m->end = start + arena_size;
	m->flags = 0;

	r->reserved = __page(reserved);
	r->start = m->start;
	r->end = m->end;
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
	for (size_t o = max_order(); o > 0; --o) {
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

	size_t ref = __page(start);
	while (n) {
		struct mem_region *t = mem_container(n);
		size_t d = ABS((ssize_t)ref - (ssize_t)t->start);

		if (d == 0) /* exact match */
			return t;

		if (d < md) { /* closest so far */
			closest = t;
			md = d;
		}

		if (ref < t->start)
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

	/* always go right, both to find a larger block and a higher
	 * address (generally avoid going towards the NULL page) */
	for (; n; n = sp_right(n)) {
		struct mem_region *t = mem_container(n);
		vm_t start = align_up(t->start, offset);
		size_t qsize = t->end - t->start;

		size_t bsize = 0;
		if (t->end >= start)
			bsize = t->end - start;

		/* handle reserved region first */
		if (start < r->start + r->reserved) {
			/* we would have to map reserved pages, go to next node
			 * if one exists */
			if (sp_right(n))
				continue;

			/* we're the only free region left to check,
			 * are we large enough to carve a chunk out of? */
			size_t offset = r->reserved - t->start;
			if (size < qsize - offset)
				return quick_best;

			/* we are, so let's set the alignment to match that at
			 * least some parts of this block should be skipped */

			/* try to use page order alignment if possible */
			if (size <= bsize - offset) {
				*align = start - t->start;
				return t;
			}

			/* otherwise, carve out a block at the top of this node. */
			*align = t->end - size;
			return t;
		}

		if (!quick_best && size <= qsize)
			quick_best = t;

		if (size <= bsize) {
			*align = start - t->start;
			return t;
		}
	}

	return quick_best;
}

struct mem_region *find_first_region(struct mem_region_root *r)
{
	/* get used region with smallest address, likely also close to the start
	 * of the linked list */
	struct mem_region *m = find_closest_used_region(r, 0);
	if (!m)
		return NULL;

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
 * @param pid Process ID to associate with region if shared. 0 if private.
 * from start of free region.
 * @param flags Flags of used region.
 * @return Start address of used region.
 */
static vm_t __partition_region(struct mem_region_root *r, struct mem_region *m,
                               size_t pages, size_t align, vmflags_t flags,
                               id_t pid)
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
	m->pid = pid;
	if (m->pid == 0)
		m->refcount = 1;

	mark_region_used(m->flags);
	__insert_used_region(r, m);
	return __addr(start);
}

vm_t alloc_shared_region(struct mem_region_root *r, size_t size,
                         size_t *actual_size,
                         vmflags_t flags, id_t pid)
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

	return __partition_region(r, m, pages, align, flags, pid);
}

vm_t alloc_region(struct mem_region_root *r, size_t size, size_t *actual_size,
                  vmflags_t flags)
{
	return alloc_shared_region(r, size, actual_size, flags, 0);
}

vm_t alloc_shared_fixed_region(struct mem_region_root *r, vm_t start,
                               size_t size, size_t *actual_size,
                               vmflags_t flags, id_t pid)
{
	size_t asize = align_up(size, BASE_PAGE_SIZE);
	if (actual_size)
		*actual_size = asize;

	size_t pages = __page(asize);
	size_t ref = __page(start);

	struct mem_region *m = find_closest_used_region(r, start);
	if (!m)
		return 0;

	/* locate actual region where start is between the region start and end */
	while (!((m->start <= ref) && (ref < m->end))) {
		if (ref > m->start)
			m = m->next;
		else
			m = m->prev;

		assert(m);
	}

	/* if region is already in use, forget it */
	if (is_region_used(m))
		return 0;

	/* region is too small */
	if (ref + pages > m->end)
		return 0;

	/* actually start marking region used */
	return __partition_region(r, m, pages, ref - m->start, flags, pid);
}

vm_t alloc_fixed_region(struct mem_region_root *r, vm_t start, size_t size,
                        size_t *actual_size, vmflags_t flags)
{
	return alloc_shared_fixed_region(r, start, size, actual_size, flags, 0);
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
	/** @todo might free mem and then reuse it, not good */
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

	free_known_region(r, m);
	return OK;
}

void free_known_region(struct mem_region_root *r, struct mem_region *m)
{
	sp_remove(&sp_root(&r->used_regions), &m->sp_n);
	mark_region_unused(m->flags);

	__try_coalesce_regions(r, m);
	__insert_free_region(r, m);
}

/**
 * Align region starting at \p start of size \p bytes to start and end on
 * BASE_PAGE boundaries. Place new start and size into \p startp and \p bytesp.
 *
 * @param start Start of region.
 * @param bytes Size of region.
 * @param startp Where to place new start.
 * @param bytesp Where to place new size.
 */
static void align_region(vm_t start, size_t bytes, vm_t *startp, size_t *bytesp)
{
	size_t shift = order_shift(BASE_PAGE);
	vm_t top = start + bytes;
	/* reasonably fast align down */
	vm_t new_start = (start >> shift) << shift;

	/* to align up, we must first align down */
	vm_t new_top = ((top >> shift) << shift);

	/* if alignment did something, add a base page size to align up */
	if (new_top != top)
		new_top += BASE_PAGE_SIZE;

	/* difference between top and start */
	size_t new_bytes = new_top - new_start;

	*startp = new_start;
	*bytesp = new_bytes;
}

/* assuming start is chosen to start on an aligned border, this should choose
 * the 'optimal' fit for the mapping.
 *
 * NOTE: not actually optimal, this doesn't bother to go through possible
 * permutations etc. which would be slow and I don't want to implement it.
 */
stat_t map_region(struct vmem *b, vm_t start, size_t bytes, enum mm_order order,
                  vmflags_t flags)
{
	/* adjust to nearest page sizes */
	align_region(start, bytes, &start, &bytes);

	size_t size = order_size(order);
	while (bytes) {
		if (size > bytes)
			goto next_order;

		/* NULL does pass this check, so technically all NULL pages are
		 * aligned, but they're caught in the while expr so this should
		 * work even if someone tries to map NULL */
		if (!is_aligned(start, size))
			goto next_order;

		pm_t page = alloc_page(order);
		if (!page)
			goto next_order;

		stat_t res = map_vpage(b, page, start, flags, order);
		if (res)
			goto next_order;


		start += size;
		bytes -= size;
		continue;

next_order:
		/* ran out of orders, stop */
		if (order == 0)
			return ERR_MISC;

		order--;
		size = order_size(order);
	}

	return OK;
}

stat_t map_fixed_region(struct vmem *b, vm_t v, pm_t start, size_t bytes,
                        vmflags_t flags)
{
	/* adjust to nearest page sizes, generally the region should be on a
	 * BASE_PAGE boundary but just to be safe */
	v = align_down(v, BASE_PAGE_SIZE);
	align_region(start, bytes, &start, &bytes);

	size_t size = BASE_PAGE_SIZE;
	while (bytes) {
		stat_t ret = map_vpage(b, start, v, flags, BASE_PAGE);
		if (ret)
			return ret;

		start += size;
		bytes -= size;
		v += size;
	}

	return OK;
}

stat_t clone_region(struct vmem *b, struct vmem *g, vm_t from, vm_t to,
                    size_t bytes, vmflags_t flags)
{
	size_t from_size = 0; size_t to_size = 0;
	align_region(from, bytes, &from, &from_size);
	align_region(to, bytes, &to, &to_size);

	assert(from_size == to_size);
	bytes = from_size;

	while (bytes) {
		pm_t addr = 0;
		enum mm_order order = BASE_PAGE;
		stat_t res = stat_vpage(g, from, &addr, &order, NULL);
		if (res)
			return res;

		res = map_vpage(b, addr, to, flags, order);
		if (res)
			return res;

		size_t size = order_size(order);
		bytes -= size;
		from += size;
		to += size;
	}

	return OK;
}

stat_t copy_region(struct vmem *b, struct vmem *g, vm_t from, vm_t to,
                   size_t bytes)
{
	size_t from_size = 0; size_t to_size = 0;
	align_region(from, bytes, &from, &from_size);
	align_region(to, bytes, &to, &to_size);

	assert(from_size == to_size);
	bytes = from_size;

	while (bytes) {
		pm_t addr = 0;
		vmflags_t flags = 0;
		enum mm_order order = BASE_PAGE;
		stat_t res = stat_vpage(g, from, &addr, &order, &flags);
		if (res)
			return res;

		pm_t page = alloc_page(order);
		if (!page)
			return ERR_OOMEM;

		/* temporarily give us write permissions */
		res = map_vpage(b, page, to, flags, order);
		if (res) {
			free_page(order, page);
			return res;
		}

		size_t size = order_size(order);
		memcpy((void *)page, (void *)addr, size);
		bytes -= size;
		from += size;
		to += size;
	}

	return OK;
}

void unmap_region(struct vmem *b, vm_t v, size_t bytes)
{
	v = align_down(v, BASE_PAGE_SIZE);
	bytes = align_up(v + bytes, BASE_PAGE_SIZE) - v;
	while (bytes) {
		pm_t addr = 0;
		enum mm_order order = BASE_PAGE;
		stat_t res = stat_vpage(b, v, &addr, &order, NULL);
		if (res)
			return;

		unmap_vpage(b, v);
		free_page(order, addr);
		size_t size = order_size(order);
		bytes -= size;
		v += size;
	}
}

void unmap_fixed_region(struct vmem *b, vm_t v, size_t bytes)
{
	v = align_down(v, BASE_PAGE_SIZE);
	bytes = align_up(v + bytes, BASE_PAGE_SIZE) - v;
	while (bytes) {
		pm_t addr = 0;
		enum mm_order order = BASE_PAGE;
		stat_t res = stat_vpage(b, v, &addr, &order, NULL);
		if (res)
			return;

		unmap_vpage(b, v);
		size_t size = order_size(order);
		bytes -= size;
		v += size;
	}
}
