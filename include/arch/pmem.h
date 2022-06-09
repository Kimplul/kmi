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

/**
 * Get physical memory parameters.
 *
 * The kernel assumes all virtual addresses can be represented approximately as
 * follows:
 *
 * @code
 * Content ... 0 1 1 0 0 1 0 1 0
 * -----------------------------
 * Index   ... 9 8 7 6 5 4 3 2 1
 *           --- --- --- +++++++
 *           O2  O1  O0  Base
 * @endcode
 *
 * Where \c Base is a portion of the address that is directly passed through
 * without any lookups, and each \c O0, \c O1, \c O2 are increasingly higher
 * order pages, where each order is an index into the higher order.
 *
 * For example, in the diagram above:
 * The base page is four bits, and each page order consists of four pages of
 * one order lower (two bits), and there are three orders, so
 * @code
 * max_order = 2
 * base_bits = 4
 * bits[0] = 2
 * bits[1] = 2
 * bits[2] = 2
 * bits[3] = 0
 * ...
 * @endcode
 *
 * \todo Move documentation into its own .md
 *
 * @param fdt Global FDT pointer.
 * @param max_order Highest order pages the arch supports.
 * @param base_bits Size of base page as number of bits in bitmask.
 * Eg. 12bits -> 4k page.
 * @param bits Size in bits as bitmask of corresponding page order. Zeroed out
 * beforehand.
 * @return \ref OK when query succesful, arch specific otherwise.
 */
stat_t stat_pmem_conf(void *fdt, size_t *max_order, size_t *base_bits,
                      size_t bits[NUM_ORDERS]);

#endif /* APOS_ARCH_PMEM_H */
