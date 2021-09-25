#ifndef APOS_RISCV_PAGES_H
#define APOS_RISCV_PAGES_H

#include <apos/types.h>
#include <apos/pmem.h>

/* assume riscv64 for now */
typedef uint64_t pm_t;

#define MM_KPAGE MM_O0
#define MM_MPAGE MM_O1
#define MM_GPAGE MM_O2
#define MM_TPAGE MM_O3

#define MM_KPAGE_SIZE SZ_4K
#define MM_MPAGE_SIZE SZ_2M
#define MM_GPAGE_SIZE SZ_1G
#define MM_TPAGE_SIZE SZ_512G

#endif /* APOS_RISCV_PAGES_H */
