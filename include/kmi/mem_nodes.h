/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_MM_NODES_H
#define KMI_MM_NODES_H

/**
 * @file mem_nodes.h
 * Memory node subsystem. Used by the memory region subsystem.
 */

#include <kmi/vmem.h>
#include <kmi/nodes.h>

/**
 * Initialize memory node subsystem.
 *
 * @pre Physical memory subsystem has been initialized.
 */
void init_mem_nodes();

/**
 * Destroy memory node subsystem.
 * Free all associated allocations.
 */
void destroy_mem_nodes();

/**
 * Fetch a new memory node.
 *
 * @return Pointer to \ref mem_region node on success, \c NULL otherwise.
 */
struct mem_region *get_mem_node();

/**
 * Free a memory node.
 *
 * @param m Pointer to \ref mem_region node to free.
 */
void free_mem_node(struct mem_region *m);

#endif /* KMI_MM_NODES_H */
