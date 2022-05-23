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

stat_t setup_arch(void *fdt);

#endif /* APOS_ARCH_H */
