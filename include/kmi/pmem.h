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
 * Page is returned as a virtual address within the kernel address space, and
 * can be converted to a physical address via the \ref __pa() macro.
 *
 * @param order Order of page to allocate.
 * @return pm_t Physical address of page when succesful, else \c NULL.
 */
pm_t alloc_page(enum mm_order order);

/** @return How many bytes of memory are currently in use. */
size_t query_used();

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
 * @param load_addr Where kernel was loaded to. Important for making sure that
 * nothing gets accidentally overwritten.
 */
void init_pmem(void *fdt, uintptr_t load_addr);

#endif /* KMI_PMEM_H */
