#ifndef APOS_MM_NODES_H
#define APOS_MM_NODES_H

/**
 * @file mem_nodes.h
 * Memory node subsystem. Used by the memory region subsystem.
 */

#include <apos/vmem.h>
#include <apos/nodes.h>

void init_mem_blocks();
void destroy_mem_blocks();

struct mem_region *get_mem_node();
void free_mem_node(struct mem_region *m);

#endif /* APOS_MM_NODES_H */
