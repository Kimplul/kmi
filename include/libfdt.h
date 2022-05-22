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
struct cell_info {
	uint32_t size_cells;
	uint32_t addr_cells;
};

struct cell_info get_cellinfo(const void *fdt, const int offset);
struct cell_info get_reginfo(const void *fdt, const char *path);

#if defined(DEBUG)
void __dbg_fdt(const void *fdt, int node_offset, int depth);
#define dbg_fdt(fdt) __dbg_fdt(fdt, 0, 0)
#else
#define dbg_fdt(...)
#endif

#define fdt_load_int32_ptr(p) fdt32_to_cpu(get_unaligned((uint32_t *)p))

#define fdt_load_int64_ptr(p) fdt64_to_cpu(get_unaligned((uint64_t *)p))

#define fdt_load_int_ptr(c, p)                                                 \
	((c) == 2 ? fdt_load_int64_ptr(p) : fdt_load_int32_ptr(p))

#endif
