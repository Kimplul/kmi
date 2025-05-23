/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_ARCH_PROC_H
#define KMI_ARCH_PROC_H

/**
 * @file proc.h
 * Arch-specific process related stuff.
 *
 * Note that all functions except *_fast may not assume that the rpc stack is
 * mapped into the current address space, so they might have to do some extra
 * calculations to find the underlying physical stack or something along those
 * lines.
 */

#if defined(__riscv)
# if __riscv_xlen == 64
#include "../../arch/riscv64/include/proc.h"
# else
#include "../../arch/riscv32/include/proc.h"
# endif
#endif

#include <kmi/uapi.h>

/**
 * Attach argument data to thread, to be returned to userspace.
 *
 * @param t Thread that will run after return.
 * @param n How many of the arguments to attach. Might micro-optimize some
 * stuff.
 * @param a Arguments to attach.
 */
void set_ret(struct tcb *t, size_t n, struct sys_ret a);

/**
 * Attach argument data to thread, to be returned to userspace.
 * Must only be called when cur_tcb() == t. Currently only used by do_ipc() so
 * no need to figure out how many registers to set, just set them all.
 *
 * @param t Current thread.
 * @param a Values to place in return registers.
 */
void set_ret_fast(struct tcb *t, struct sys_ret a);

/**
 * Get argument data attached to thread.
 * To some extent a hack, used by swap.
 *
 * @todo is swap necessary? Would assign be enough?
 *
 * @param t Thread to read data from.
 * @return Args associated with thread.
 */
struct sys_ret get_ret(struct tcb *t);

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
 * Set userspace stack. I think \ref set_thread() could be replaced with this,
 * and it's more useful.
 *
 * @param t Thread whose stack to set.
 * @param s Stack to give to thread.
 */
void set_stack(struct tcb *t, vm_t s);

/**
 * Set userspace stack, but do it very quickly.
 * Must only be called when cur_tcb() == t, allowing us to do a fast but 'unsafe'
 * direct write to the rpc stack.
 *
 * @param t Current thread.
 * @param s Address to place into the stack register.
 */
void set_stack_fast(struct tcb *t, vm_t s);

/**
 * Get current userspace stack.
 *
 * @param t Thread whose stack to query.
 * @return Current stack address.
 */
vm_t get_stack(struct tcb *t);

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
 * Copy registers from one thread to another. Used by \ref fork(), for example.
 *
 * @param d Destination.
 * @param s Source.
 */
void copy_regs(struct tcb *d, struct tcb *s);

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
 * @param initrd Pointer to initrd that is passed to \c init.
 */
__noreturn void run_init(struct tcb *t, vm_t fdt, vm_t initrd);

/**
 * Return to userspace fast.
 * Doesn't load registers besides arguments.
 * Mainly for do_ipc().
 * Note that this skips canary checking,
 * but at least do_ipc() uses basically zero stack space
 * so I'm not too worried.
 */
__noreturn void ret_userspace_fast();

/**
 * Return to userspace, restoring some registers.
 * Mainly for returning from an ipc_resp, where the user is expected to save
 * temporary registers on their own.
 * Same as with \ref ret_userspace_fast(), skips canary checking.
 */
__noreturn void ret_userspace_partial();

#endif /* KMI_ARCH_PROC_H */
