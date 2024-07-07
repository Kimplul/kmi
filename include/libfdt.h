/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_LIBFDT_H
#define KMI_LIBFDT_H

/**
 * @file libfdt.h
 * Main include file in kmi for functions in libfdt.
 */

#include "../dtc/libfdt/libfdt.h"
#include <kmi/unaligned.h>
#include <kmi/assert.h>
#include <kmi/types.h>

/* kmi additions, implementation can be found in common/fdt.c */

/**
 * FDT cell info.
 */
struct cell_info {
	/** Value size of cell. */
	uint32_t size_cells;
	/** Address size of cell. */
	uint32_t addr_cells;
};

/**
 * Get information about a cell.
 *
 * @param fdt Pointer to the global FDT.
 * @param offset Offset of cell to poke.
 * @return Cell information.
 */
struct cell_info get_cellinfo(const void *fdt, const int offset);

#if defined(DEBUG)
/**
 * Print FDT node at specified location.
 *
 * @param fdt Pointer to global FDT.
 * @param node_offset Offset of node.
 * @param depth Depth of node.
 */
void __dbg_fdt(const void *fdt, int node_offset, int depth);

/**
 * Convenience wrapper around \ref __dbg_fdt(), used to print whole tree.
 */
#define dbg_fdt(fdt) __dbg_fdt(fdt, 0, 0)
#else
/**
 * Debugging is disabled when in release mode, so all calls to dbg_fdt need to
 * be erased.
 */
#define dbg_fdt(...)
#endif

/**
 * Load int32 from FDT at location specified by pointer. Integers inside the FDT
 * may be unaligned, and accessing them should preferably be done through this
 * and \ref fdt_load_int64_ptr().
 *
 * @param p Pointer to int32 inside the global FDT.
 */
#define fdt_load_int32_ptr(p) fdt32_to_cpu(get_unaligned((uint32_t *)p))

/**
 * Load int64 from FDT at location specified by pointer.
 *
 * @param p Pointer to int64 inside the global FDT.
 * @return Value of int32 at location p.
 *
 * See \ref fdt_load_int32_ptr()
 */
#define fdt_load_int64_ptr(p) fdt64_to_cpu(get_unaligned((uint64_t *)p))

/**
 * Load int{32,64} from FDT at location specified by pointer.
 *
 * @param c Size of int, where 2 == int64 and everything else int32. Query int
 * size from FDT with \ref get_cellinfo().
 * @param p Pointer to int{32,64} inside the global FDT.
 * @return Value of integer at location p.
 */
#define fdt_load_int_ptr(c, p) \
	((c) == 2 ? fdt_load_int64_ptr(p) : fdt_load_int32_ptr(p))

/**
 * Helper for reading address value from reg nodes.
 * No larger than 64 bit values allowed.
 *
 * @param ci Cell info.
 * @param p Pointer to node.
 * @param i Index of address to read.
 * @return Address in reg cell.
 */
static inline fdt64_t fdt_load_reg_addr(struct cell_info ci, const void *p,
                                        size_t i)
{
	assert(ci.addr_cells == 2 || ci.addr_cells == 1);
	size_t offset = i * (ci.addr_cells + ci.size_cells) * sizeof(fdt32_t);
	char *addr = ((char *)p) + 0;
	return fdt_load_int_ptr(ci.addr_cells, addr + offset);
}

/**
 * Helper for reading the size cell of a reg node.
 *
 * @param ci Cell info.
 * @param p Pointer to node.
 * @param i Index of size to read.
 * @return Size in reg cell.
 */
static inline fdt64_t fdt_load_reg_size(struct cell_info ci, const void *p,
                                        size_t i)
{
	assert(ci.size_cells == 2 || ci.size_cells == 1);
	size_t offset = i * (ci.addr_cells + ci.size_cells) * sizeof(fdt32_t);
	char *addr = ((char *)p) + ci.addr_cells * sizeof(fdt32_t);
	return fdt_load_int_ptr(ci.size_cells, addr + offset);
}

#endif
