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

/**
 * Set address to jump to when returning to userspace.
 *
 * @param r Address to jump to.
 * @return \ref OK.
 */
stat_t set_return(vm_t r);

/**
 * Attach IPC data to load into argument registers when returning to userspace.
 *
 * @param t Thread that will run after return.
 * @param pid Process ID.
 * @param tid Thread ID.
 * @return \ref OK.
 */
stat_t set_ipc(struct tcb *t, id_t pid, id_t tid);

/** \todo Should these be in arch/tcb.h or something? */
/** \todo Should these be void? */

/**
 * Attach thread stack and thread local storage when returning to userspace.
 *
 * Their info should already be stored in \c t, and this function just
 * actualizes the information.
 *
 * @param t Thread that will run after return.
 * @return \ref OK.
 */
stat_t set_thread(struct tcb *t);

/**
 * Run \c init program.
 *
 * @param t Thread that \c init is attached to.
 * @param fdt Pointer to FDT that is passed to \c init.
 * @return \ref ERR_ADDR, as it shouldn't return-
 */
stat_t run_init(struct tcb *t, void *fdt);

#endif /* APOS_ARCH_PROC_H */
