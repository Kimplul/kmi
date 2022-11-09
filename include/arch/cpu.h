/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_CPU_H
#define APOS_CPU_H

/**
 * @file cpu.h
 * Arch-specific cpu handling, generally implemented in
 * arch/whatever/kernel/cpu.c
 */

#include <apos/types.h>
#include <apos/tcb.h>

#if defined(riscv64)
#include "../../arch/riscv64/include/cpu.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/cpu.h"
#endif

/**
 * Get current CPU ID.
 * ID must be in range \c 0 .. \c num_cpus, where \c num_cpus is the number of
 * cpus on this system.
 *
 * @return Current CPU ID.
 */
id_t cpu_id();

/**
 * Assign tcb to current cpu.
 * Essentially sets t->cpu_id to the cpu id, plus whatever the arch needs.
 *
 * @param t \ref tcb to assign to current cpu.
 */
void cpu_assign(struct tcb *t);

/**
 * Send inter-processor interrupt to cpu \p cpu_id.
 *
 * @param cpu_id CPU to send ipi to.
 */
void cpu_send_ipi(id_t cpu_id);

#endif /* APOS_CPU_H */
