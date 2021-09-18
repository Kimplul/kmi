#ifndef APOS_LIBFDT_H
#define APOS_LIBFDT_H
#include "../dtc/libfdt/libfdt.h"

#if defined(DEBUG)
void __dbg_fdt(void *fdt, int node_offset, int depth);
#define dbg_fdt(fdt) __dbg_fdt(fdt, 0, 0)
#else
#define dbg_fdt(...)
#endif

#endif
