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
 * @return OK on success, non-zero otherwise.
 */
stat_t setup_rpc_stack(struct tcb *t);

/**
 * Free memory backing rpc stack.
 *
 * @param t Thred whose RPC stack should be destroyed.
 */
void destroy_rpc_stack(struct tcb *t);

/**
 * Copy over contents in rpc stack from \p t to \p c.
 *
 * @param t 'Source'.
 * @param c 'Destination'-
 */
void copy_rpc_stack(struct tcb *t, struct tcb *c);

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
 * Checks whether an address is in the rpc stack.
 * Currently also serves double-duty to check that the address is currently
 * inaccessible, but could be made accessible with \ref grow_rpc. Used by
 * \ref handle_pagefault.
 *
 * @param t tcb to check.
 * @param addr Address to check.
 * @return \ref true if the above holds, \ref false otherwise.
 */
bool in_rpc_stack(struct tcb *t, vm_t addr);

/**
 * Shrinks rpc stack down to where the previous reserved area is, so stack looks
 * something like this:
 *
 * ```
 * ctx | user | <
 * v
 * ctx | <
 * ```
 *
 * Implementation note: t->arch.rpc_idx now points to ctx.
 *
 * @param t tcb to shrink.
 */
void shrink_rpc(struct tcb *t);

/**
 * Opens back up a previous stack frame, so
 * ```
 * ctx | user | ctx | <
 * v
 * ctx | user | <
 * ```
 *
 * @param t tcb to open up again.
 * @param top Address to where previous ctx is.
 */
void open_rpc(struct tcb *t, vm_t top);

/**
 * Adds more accessible space on the stack, so
 * ```
 * ctx | user | <
 * v
 * ctx | user | user | <
 * ```
 *
 * @param t tcb whose stack to grow.
 * @param top Address to grow to.
 */
void grow_rpc(struct tcb *t, vm_t top);

/**
 * Marks all user accessible areas inaccessible, so
 * ```
 * ctx | user | <
 * ctx | ---- | <
 * ```
 *
 * Used by \ref do_ipc to make sure different processes can't mess with
 * eachother's stacks.
 *
 * @param t tcb whose stack should be closed.
 */
void close_rpc(struct tcb *t);

#endif /* KMI_ARCH_TCB_H */
