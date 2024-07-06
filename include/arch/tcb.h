/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_ARCH_TCB_H
#define KMI_ARCH_TCB_H

/**
 * @file lock.h
 * Arch-specific tcb stuff.
 */

#if defined(__riscv)
# if __riscv_xlen == 64
#include "../../arch/riscv64/include/tcb.h"
# else
#include "../../arch/riscv32/include/tcb.h"
# endif
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

/**
 * Set up RPC stack in a way that is convenient for the underlying architecture.
 *
 * @param t Thread whose RPC stack should be set up.
 */
void setup_rpc_stack(struct tcb *t);

/**
 * Free memory backing rpc stack.
 *
 * @param t Thred whose RPC stack should be destroyed.
 */
void destroy_rpc_stack(struct tcb *t);

/**
 * Reset RPC stack to top.
 *
 * @param t Thread whose RPC stack should be reset.
 */
void reset_rpc_stack(struct tcb *t);

/**
 * @return Max size of one individual RPC stack instance.
 */
size_t max_rpc_size();

/**
 * @param addr Address of current top of stack, generally from `ctx->rpc_stack`.
 * @return \ref true if next rpc return would be into the root process.
 */
bool rpc_stack_empty(pm_t addr);

/**
 * Current highest address in RPC stack. Allowed to be inaccurate to one base page.
 *
 * @param t Thread whose position in the RPC stack is to be determined.
 * @return Virtual address corresponding to the current RPC stack position.
 */
vm_t rpc_position(struct tcb *t);

/**
 * Mark RPC stack up to \p top accessible from userspace.
 *
 * @param t Thread whose RPC stack is being modified.
 * @param top Address up to where stack should be accessible from userspace.
 */
void mark_rpc_valid(struct tcb *t, vm_t top);

/**
 * Mark RPC stack down to \p bottom inaccessible from userspace.
 *
 * @param t Thread whose RPC stack is being modified.
 * @param bottom Address down to where stack should be inaccessible from
 * userspace.
 */
void mark_rpc_invalid(struct tcb *t, vm_t bottom);

#endif /* KMI_ARCH_TCB_H */
