/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_PMEM_H
#define KMI_PMEM_H

/**
 * @file pmem.h
 * Physical memory subsystem. Used to allocate and free physical memory pages.
 */

#include <kmi/mem.h>
#include <kmi/types.h>
#include <arch/pmem.h>

/**
 * Free physical page.
 *
 * @param order Order of page to free.
 * @param addr Physical address of page.
 */
void free_page(enum mm_order order, pm_t addr);

/**
 * Mark page used.
 *
 * @param order Order of page to mark.
 * @param addr Physical address of page.
 */
void mark_used(enum mm_order order, pm_t addr);

/**
 * Allocate physical page.
 * Allows the user to specify a hint as to which address to start looking for.
 * Useful for allocating many pages for one virtual allocation, for example, and
 * allows us to skip already checked pages when allocating a second page.
 *
 * @param order Order of page to allocate.
 * @return pm_t Physical address of page when succesful, else \c NULL.
 */
pm_t alloc_page(enum mm_order order);

/**
 * Populate physical RAM usage map.
 * In theory we could easily implement NUMA nodes by just using different orders
 * of usage maps, but for now we assume all RAM is contiguous.
 *
 * @param ram_base Base physical address of RAM.
 * @param ram_size Size of physical RAM.
 * @param cont Physical address where to place the map.
 * @return Size of physical map. Check that is matches with \ref probe_pmap().
 */
size_t populate_pmap(pm_t ram_base, size_t ram_size, pm_t cont);

/**
 * Probe size of RAM usage map.
 * 'Pretends' to be populate_pmap().
 *
 * @param ram_base Base physical address RAM.
 * @param ram_size Size of physical RAM.
 * @param cont Physical address where to place the map.
 * @return Size of physical map. Check that it matches with \ref
 * populate_pmap().
 */
size_t probe_pmap(pm_t ram_base, size_t ram_size, pm_t cont);

/**
 * Initialize physical memory subsystem.
 *
 * @param fdt Global FDT pointer.
 */
void init_pmem(void *fdt);

#endif /* KMI_PMEM_H */
