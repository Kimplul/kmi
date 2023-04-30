/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file mem_nodes.c
 * Memory node wrapper around the node subsystem, used by \ref
 * common/mem_regions.c.
 *
 * Each region of memory is allocated through a \ref mem_region node, which is
 * allocated through the node subsystem.
 */

#include <kmi/vmem.h>
#include <kmi/pmem.h>
#include <kmi/mem.h>
#include <kmi/string.h>
#include <kmi/mem_nodes.h>

/** Memory node subsystem instance. */
static struct node_root root;

void init_mem_nodes()
{
	init_nodes(&root, sizeof(struct mem_region));
}

void destroy_mem_nodes()
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
