/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2024, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_ARCH_MEM_H
#define KMI_ARCH_MEM_H

/**
 * @file mem.h
 *
 * Arch-specific stuff generic to all memory.
 */

#if defined(__riscv)
# if __riscv_xlen == 64
#include "../../arch/riscv64/include/pmem.h"
# else
#include "../../arch/riscv32/include/pmem.h"
# endif
#endif

#endif /* KMI_ARCH_MEM_H */
