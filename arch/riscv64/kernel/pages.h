#ifndef APOS_RISCV_PAGES_H
#define APOS_RISCV_PAGES_H

/**
 * @file pages.h
 * riscv64-specific shorthands for some page sizes.
 */

/** riscv64 4KiB page, order 0. */
#define MM_KPAGE      MM_O0

/** riscv64 2MiB page, order 1. */
#define MM_MPAGE      MM_O1

/** riscv64 1GiB page, order 2. */
#define MM_GPAGE      MM_O2

/** riscv64 512GiB page, order 3. */
#define MM_TPAGE      MM_O3

/** riscv64 4KiB page size. */
#define MM_KPAGE_SIZE SZ_4K

/** riscv64 2MiB page size. */
#define MM_MPAGE_SIZE SZ_2M

/** riscv64 1GiB page size. */
#define MM_GPAGE_SIZE SZ_1G

/** riscv64 512GiB page size. */
#define MM_TPAGE_SIZE SZ_512G

#endif /* APOS_RISCV_PAGES_H */
