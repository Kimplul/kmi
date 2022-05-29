#ifndef APOS_LIBFDT_H
#define APOS_LIBFDT_H

/**
 * @file libfdt.h
 * Main include file in apos for functions in libfdt.
 */

#include "../dtc/libfdt/libfdt.h"
#include <apos/unaligned.h>
#include <apos/types.h>

/* apos additions, implementation can be found in common/fdt.c */

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

/**
 * Get information about a register.
 *
 * @param fdt Pointer to the global fdt.
 * @param path Path to be searched.
 * @return Register information.
 */
struct cell_info get_reginfo(const void *fdt, const char *path);

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
 * size from FDT with \ref get_cellinfo() and \ref get_reginfo().
 * @param p Pointer to int{32,64} inside the global FDT.
 * @return Value of integer at location p.
 */
#define fdt_load_int_ptr(c, p) \
	((c) == 2 ? fdt_load_int64_ptr(p) : fdt_load_int32_ptr(p))

#endif
