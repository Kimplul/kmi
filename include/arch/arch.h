/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_ARCH_H
#define KMI_ARCH_H

/**
 * @file arch.h
 * Arch-specific generic stuff, generally implemented in
 * arch/whatever/kernel/main.c
 */

#include <kmi/types.h>

#if defined(riscv)
# if __riscv_xlen == 64
#include "../../arch/riscv64/include/arch.h"
# else
#include "../../arch/riscv32/include/arch.h"
# endif
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

#endif /* KMI_ARCH_H */
