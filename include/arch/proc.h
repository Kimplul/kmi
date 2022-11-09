/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_ARCH_PROC_H
#define APOS_ARCH_PROC_H

/**
 * @file proc.h
 * Arch-specific process related stuff.
 */

#if defined(riscv64)
#include "../../arch/riscv64/include/proc.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/proc.h"
#endif

#include <apos/uapi.h>

/**
 * Attach argument data to thread.
 *
 * @param t Thread that will run after return.
 * @param a Arguments to attach.
 */
void set_args(struct tcb *t, struct sys_ret a);

/**
 * Get argument data attached to thread.
 * To some extent a hack, used by swap.
 *
 * @todo is swap necessary? Would assign be enough?
 *
 * @param t Thread to read data from.
 * @return Args associated with thread.
 */
struct sys_ret get_args(struct tcb *t);

/** \todo Should these be in arch/tcb.h or something? */

/**
 * Attach thread stack and thread local storage when returning to userspace.
 *
 * Their info should already be stored in \c t, and this function just
 * actualizes the information.
 *
 * @param t Thread that will run after return.
 */
void set_thread(struct tcb *t);

/**
 * Copy registers from tcb save area to address \p p.
 * Intended to be used for copying thread state to rpc stack.
 *
 * @param t Thread whose registers to save.
 * @param p Address to save to.
 */
void save_regs(struct tcb *t, void *p);

/**
 * Copy registers from address \p p to tcb save registers.
 * Intended to be used for copying thread state from rpc stack.
 *
 * @param p Address to load from.
 * @param t Thread whose registers to load.
 */
void load_regs(void *p, struct tcb *t);

/**
 * Do modifications to \ref tcb state if necessary for ipis to work.
 *
 * @param t Thread to do modifications to.
 */
void adjust_ipi(struct tcb *t);

/**
 * Do modifications to \ref tcb state if necessary for syscalls to work.
 *
 * @param t Thread to do modifications to.
 */
void adjust_syscall(struct tcb *t);

/**
 * Run \c init program.
 *
 * @param t Thread that \c init is attached to.
 * @param fdt Pointer to FDT that is passed to \c init.
 */
__noreturn void run_init(struct tcb *t, void *fdt);

#endif /* APOS_ARCH_PROC_H */
