#ifndef MM_NODES_H
#define MM_NODES_H

#include <apos/vmem.h>

void init_mem_blocks();
void destroy_mem_blocks();

struct mem_region *get_mem_node();
void free_mem_node(struct mem_region *m);

#endif /* MM_NODES_H */
