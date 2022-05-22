/**
 * @file mem_nodes.c
 * Memory node wrapper around the node subsystem, used by \ref
 * common/mem_regions.c.
 *
 * Each region of memory is allocated through a \ref mem_region node, which is
 * allocated through the node subsystem.
 */

#include <apos/vmem.h>
#include <apos/pmem.h>
#include <apos/mem.h>
#include <apos/string.h>
#include <apos/mem_nodes.h>

static struct node_root root;

void init_mem_blocks()
{
	init_nodes(&root, sizeof(struct mem_region));
}

void destroy_mem_blocks()
{
	destroy_nodes(&root);
}

struct mem_region *get_mem_node()
{
	return (struct mem_region *)get_node(&root);
}

void free_mem_node(struct mem_region *m)
{
	free_node(&root, (void *)m);
}
