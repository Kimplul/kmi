#ifndef APOS_ARCH_PMEM_H
#define APOS_ARCH_PMEM_H

/**
 * @file pmem.h
 * Arch-specific physical memory handling, generally implemented in
 * arch/whatever/kernel/pmem.c
 */

#include <apos/mem.h> /* NUM_ORDERS */
#include <apos/types.h>

#if defined(riscv64)
#include "../../arch/riscv64/include/pmem.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/pmem.h"
#endif

stat_t stat_pmem_conf(void *fdt, size_t *max_order, size_t *base_bits,
                      size_t bits[NUM_ORDERS]);

#endif /* APOS_ARCH_PMEM_H */
