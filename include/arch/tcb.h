/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_ARCH_TCB_H
#define KMI_ARCH_TCB_H

/**
 * @file lock.h
 * Arch-specific tcb stuff.
 */

#if defined(riscv64)
#include "../../arch/riscv64/include/tcb.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/tcb.h"
#endif

/**
 * Force store tcb \p t in arch-specific way.
 * cpu_assign() is allowed to assume there's always a valid tcb
 * set, so during init we have to force set a tcb before cpu_assign() can be
 * used.
 *
 * @param t tcb to set.
 */
void tcb_assign(struct tcb *t);

#endif /* KMI_ARCH_TCB_H */
