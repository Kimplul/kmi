#ifndef APOS_LIBFDT_H
#define APOS_LIBFDT_H
#include "../dtc/libfdt/libfdt.h"

/* apos additions, implementation can be found in common/fdt.c */
struct cell_info_t {
	uint32_t size_cells;
	uint32_t addr_cells;
};

struct cell_info_t get_cellinfo(void *fdt, int offset);
struct cell_info_t get_reginfo(void *fdt, const char *path);

#if defined(DEBUG)
void __dbg_fdt(void *fdt, int node_offset, int depth);
#define dbg_fdt(fdt) __dbg_fdt(fdt, 0, 0)
#else
#define dbg_fdt(...)
#endif

#define fdt_load_int_ptr(c, p)\
	((c) == 2 ? fdt64_to_cpu(*(fdt64_t *)(p)) : fdt32_to_cpu(*(fdt32_t *)(p)))

#endif
