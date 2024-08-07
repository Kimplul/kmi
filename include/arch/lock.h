/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_ARCH_LOCK_H
#define KMI_ARCH_LOCK_H

/**
 * @file lock.h
 * Arch-specific lock stuff.
 */

#if defined(__riscv)
# if __riscv_xlen == 64
#include "../../arch/riscv64/include/lock.h"
# else
#include "../../arch/riscv32/include/lock.h"
# endif
#endif

#endif /* KMI_ARCH_LOCK_H */
