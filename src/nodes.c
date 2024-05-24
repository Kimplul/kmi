/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file nodes.c
 * The node subsystem. Each client has to initialize their own node system,
 * after which they can request nodes of a size specified at init.
 *
 * Node allocation is implemented through a similar system used by jemalloc
 * (https://github.com/jemalloc/jemalloc), but instead of having a number of
 * different sized buckets there is only the one size specified by the user.
 * This cuts down on complexity and improves performance, at a somewhat major
 * flexibility cost. Still, this kernel generally only allocates nodes of the
 * same size again and again, and this approach seems sensible.
 *
 *
 * Quick overview of the allocator, all nodes live in memory pages. Each memory
 * page has a small header at the front, with some metadata about number of free
 * and used node slots. When a memory page is filled, a new one is allocated by
 * the physical memory subsystem and the pages are linked together in a common
 * list. At the same time, a second linked list is maintained which maintains
 * which pages have empty slots. When a node is freed, the page it belonged to
 * is added to the free list (if it didn't already exist there) and when a new
 * node is requested, the free list is looked through first.
 *
 * \todo More in-depth documentation about the node algorithm.
 */

#include <kmi/mem.h>
#include <kmi/pmem.h>
#include <kmi/bits.h>
#include <kmi/nodes.h>
#include <kmi/string.h>

/* the structure of each node_region is approximately
 *
 * struct node_region | bitmap | array of node_size nodes
 *
 * where array starts on a multiple of node_size to ensure alignment and
 * bitmap is a bitmap of whether node at index is free or used (1 being used, 0
 * being free)
 */

/**
 * Get start of node region from pointer.
 *
 * @param r Pointer to node inside node region.
 * @return Corresponding node region.
 */
#define node_region(r) \
	((struct node_region *)((uintptr_t)(r) & ~(BASE_PAGE_SIZE - 1)))

/**
 * Create new node region.
 *
 * @return Pointer to created region.
 */
static struct node_region *__create_region()
{
	struct node_region *r = (struct node_region *)alloc_page(BASE_PAGE);
	memset(r, FREE, BASE_PAGE_SIZE);
	return r;
}

void init_nodes(struct node_root *r, size_t node_size)
{
	r->head = __create_region();
	r->av_head = r->head;
	r->node_size = node_size;
	r->bitmap = sizeof(struct node_region);

	/* ideal values */
	size_t max_nodes = BASE_PAGE_SIZE / node_size;
	/* make sure not to truncate division */
	size_t bitmap_size = (max_nodes / 8) + 1;
	uintptr_t first_node = r->bitmap + bitmap_size;
	/* actual values */
	r->first_node = align_up(first_node, node_size);
	r->max_nodes = max_nodes - (r->first_node / node_size);
}

void destroy_nodes(struct node_root *r)
{
	struct node_region *nr = r->head;
	while (nr) {
		struct node_region *d = nr;
		nr = nr->prev;
		free_page(BASE_PAGE, (pm_t)d);
	}
}

/**
 * Find free node in node region.
 *
 * @param r Node root to work in.
 * @param nr Node region to look in.
 * @return Pointer to free node.
 */
static void *__find_free_node(struct node_root *r, struct node_region *nr)
{
	uint8_t *bitmap = r->bitmap + (uint8_t *)nr;
	for (size_t i = 0; i < r->max_nodes; ++i) {
		if (bitmap_is_set(bitmap, i))
			continue;

		bitmap_set(bitmap, i);
		return (i * r->node_size) + (r->first_node + (uint8_t *)nr);
	}

	return 0;
}

/**
 * Pop free list head.
 *
 * @param r Node region root to work in.
 */
static void __pop_av_head(struct node_root *r)
{
	struct node_region *t = r->av_head;
	r->av_head = r->av_head->next;
	if (r->av_head)
		r->av_head->av_prev = 0;

	t->av_next = 0;
	t->av_prev = 0;
}

void *get_node(struct node_root *r)
{
	if (!r)
		return 0;

	if (!r->av_head) {
		r->av_head = __create_region();

		r->av_head->prev = r->head;
		r->head->next = r->av_head;

		r->head = r->av_head;
	}

	void *p = __find_free_node(r, r->av_head);
	if (++r->av_head->used_nodes == r->max_nodes)
		__pop_av_head(r);

	return p;
}

/**
 * Push free list head.
 *
 * @param r Node region root to work in.
 * @param nr Node region to push.
 */
static void __push_av_head(struct node_root *r, struct node_region *nr)
{
	nr->av_prev = 0;
	nr->av_next = r->av_head;
	if (r->av_head)
		r->av_head->av_prev = nr;

	r->av_head = nr;
}

/**
 * Free a node region.
 *
 * @param r Node region root to work in.
 * @param nr Node region to free.
 */
static void __free_region(struct node_root *r, struct node_region *nr)
{
	struct node_region *av_n = nr->av_next;
	struct node_region *av_p = nr->av_prev;

	if (av_n)
		av_n->av_prev = av_p;

	if (av_p)
		av_p->av_next = av_n;

	if (nr == r->av_head)
		__pop_av_head(r);

	struct node_region *n = nr->next;
	struct node_region *p = nr->prev;

	if (n)
		n->prev = p;

	if (p)
		p->next = n;

	if (nr == r->head) {
		if (r->head->prev) {
			r->head->next = 0;
			r->head = r->head->prev;
		} else
			return;
	}

	free_page(BASE_PAGE, (pm_t)nr);
}

void free_node(struct node_root *r, void *p)
{
	struct node_region *nr = node_region(p);
	uint8_t *bitmap = r->bitmap + (uint8_t *)nr;
	size_t i =
		((uintptr_t)p - (r->first_node + (uintptr_t)nr)) / r->node_size;
	bitmap_clear(bitmap, i);

	if (--nr->used_nodes == 0) {
		__free_region(r, nr);
		return;
	}

	else if (!nr->av_next && !nr->av_prev)
		__push_av_head(r, nr);
}
