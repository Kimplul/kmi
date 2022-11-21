/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file proc.c
 * riscv64 implementation of arch-specific process handling.
 */

#include <apos/tcb.h>
#include <apos/elf.h>
#include <apos/string.h>

#include <arch/proc.h>

#include "regs.h"
#include "csr.h"

void run_init(struct tcb *t, void *fdt)
{
	/** \todo actually map fdt into the target address space */
	csr_write(CSR_SSCRATCH, t);
	csr_write(CSR_SEPC, t->exec);
	/* gcc gives a warning 'the value of the stack pointer after an asm
	 * statement must be the same as it was before the statement', so this
	 * is technically speaking undefined behavior, I think.
	 *
	 * Could be fixed with a separate pure asm run_init, but I guess this
	 * works for now.
	 */
	__asm__ volatile ("mv sp, %0\n" : : "r" (t->thread_stack_top) : "memory");
	__asm__ volatile ("mv a0, %0\n" : : "r" (t->tid) : "a0");
	__asm__ volatile ("mv a1, %0\n" : : "r" (fdt) : "a1");
	__asm__ volatile ("sret\n" ::: "memory");
	/* we should never reach this */
	unreachable();
}

void set_args(struct tcb *t, struct sys_ret a)
{
	struct riscv_regs *r = (struct riscv_regs *)(t->regs) - 1;
	r->a0 = a.s;
	r->a1 = a.ar0;
	r->a2 = a.ar1;
	r->a3 = a.ar2;
	r->a4 = a.ar3;
	r->a5 = a.ar4;
}

struct sys_ret get_args(struct tcb *t)
{
	struct riscv_regs *r = (struct riscv_regs *)(t->regs) - 1;
	return SYS_RET6(r->a0, r->a1, r->a2, r->a3, r->a4, r->a5);
}

void set_thread(struct tcb *t)
{
	/* get location of registers in memory */
	/** \todo check alignment, should be fine but just to be sure */
	struct riscv_regs *r = (struct riscv_regs *)(t->regs) - 1;

	/* insert important values into register slots */
	r->sp = (long)t->thread_stack_top;
	r->tp = (long)t->thread_storage;
}

vm_t get_stack(struct tcb *t)
{
	struct riscv_regs *r = (struct riscv_regs *)(t->regs) - 1;
	return r->sp;
}

void clone_regs(struct tcb *d, struct tcb *s)
{
	struct riscv_regs *rd = (struct riscv_regs *)(d->regs) - 1;
	struct riscv_regs *rs = (struct riscv_regs *)(s->regs) - 1;
	*rd = *rs;
}

void adjust_ipi(struct tcb *t)
{
	UNUSED(t);
}

void adjust_syscall(struct tcb *t)
{
	t->exec += 4;
}
