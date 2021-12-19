#ifndef MM_NODES_H
#define MM_NODES_H

#include <apos/vmem.h>

void init_mem_blocks();
void destroy_mem_blocks();

struct sp_mem *get_mem_node();
void free_mem_node(struct sp_mem *m);

#endif /* MM_NODES_H */
