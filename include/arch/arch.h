#ifndef APOS_ARCH_H
#define APOS_ARCH_H

/**
 * @file arch.h
 * Arch-specific generic stuff, generally implemented in
 * arch/whatever/kernel/main.c
 */

#include <apos/types.h>

#if defined(riscv64)
#include "../../arch/riscv64/include/arch.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/arch.h"
#endif

/**
 * Setup arch specific stuff.
 *
 * See arch documentation if there is any.
 *
 * @param fdt Global FDT.
 * @return \ref OK if boot can continue, error otherwise.
 * \todo Check errors.
 */
stat_t setup_arch(void *fdt);

#endif /* APOS_ARCH_H */
