#ifndef APOS_NODES_H
#define APOS_NODES_H

/**
 * @file nodes.h
 * Node subsystem. Used by a number of subsystems for allocating specific sizes
 * of memory nodes smaller than memory pages.
 */

#include <apos/types.h>

enum node_status { FREE = 0, USED = 1 };
struct node_region {
	size_t used_nodes;

	struct node_region *av_next;
	struct node_region *av_prev;

	struct node_region *next;
	struct node_region *prev;
};

struct node_root {
	size_t node_size;
	size_t max_nodes;

	ptrdiff_t bitmap;
	ptrdiff_t first_node;

	struct node_region *head;
	struct node_region *av_head;
};

void init_nodes(struct node_root *r, size_t node_size);
void destroy_nodes(struct node_root *r);

void *get_node(struct node_root *r);
void free_node(struct node_root *r, void *p);

#endif /* APOS_NODES_H */
