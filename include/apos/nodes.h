#ifndef APOS_NODES_H
#define APOS_NODES_H

/**
 * @file nodes.h
 * Node subsystem. Used by a number of subsystems for allocating specific sizes
 * of memory nodes smaller than memory pages.
 */

#include <apos/types.h>

/** Node slot status. */
enum node_status {
	/** Free. */
	FREE = 0,

	/** Used. */
	USED = 1
};

/** Header for a region in memory with node slots. */
struct node_region {
	/** Number of used slots in this region.
	 * \note Total number of slots is calculated on an instance basis. */
	size_t used_nodes;

	/** Next node region in free list. */
	struct node_region *av_next;

	/** Previous node region in free list. */
	struct node_region *av_prev;

	/** Next node slot region. */
	struct node_region *next;

	/** Previous slot node region. */
	struct node_region *prev;
};

/** Node subsystem instance. */
struct node_root {
	/** Size of each node. */
	size_t node_size;
	/** Maximum number of node slots in one region. */
	size_t max_nodes;

	/** Offset of node slot state bitmap from start of region. */
	ptrdiff_t bitmap;

	/** Offset of first node from start of region. */
	ptrdiff_t first_node;

	/** List of all node regions. */
	struct node_region *head;

	/** List of node regions with free slots. */
	struct node_region *av_head;
};

/**
 * Initialize node subsystem instance.
 *
 * @param r Node region root.
 * @param node_size Size of one node.
 */
void init_nodes(struct node_root *r, size_t node_size);

/**
 * Destroy node subsystem instance.
 *
 * @param r Node region root.
 */
void destroy_nodes(struct node_root *r);

/**
 * Allocate a new node.
 *
 * @param r Node region root.
 * @return Pointer to allocated node when succesful, \c 0 otherwise.
 */
void *get_node(struct node_root *r);

/**
 * Free a node.
 *
 * @param r Node region root.
 * @param p Pointer to node to free.
 */
void free_node(struct node_root *r, void *p);

#endif /* APOS_NODES_H */
