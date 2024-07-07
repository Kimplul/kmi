/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file proc.c
 * riscv64 implementation of arch-specific process handling.
 */

#include <kmi/tcb.h>
#include <kmi/elf.h>
#include <kmi/bkl.h>
#include <kmi/string.h>

#include <arch/proc.h>

#include "regs.h"
#include "csr.h"

void run_init(struct tcb *t, vm_t fdt, vm_t initrd)
{
	csr_write(CSR_SSCRATCH, t);
	csr_write(CSR_SEPC, t->callback);
	/* gcc gives a warning 'the value of the stack pointer after an asm
	 * statement must be the same as it was before the statement', so this
	 * is technically speaking undefined behavior, I think.
	 *
	 * Could be fixed with a separate pure asm run_init, but I guess this
	 * works for now.
	 */
	vm_t stack_top = t->thread_stack + t->thread_stack_size;
	bkl_unlock();
	__asm__ volatile ("mv sp, %0\n"
	                  "li a0, %1\n"
	                  "li a1, %2\n"
	                  "mv a2, %3\n"
	                  "mv a3, %4\n"
	                  "mv a4, %5\n"
	                  "sret\n"
	                  :
	                  : "r" (stack_top),
	                  "K" (0), "K" (SYS_USER_BOOTED),
	                  "r" (t->tid), "r" (fdt), "r" (initrd)
	                  : "memory");
	/* we should never reach this */
	unreachable();
}

void set_args(struct tcb *t, size_t n, struct sys_ret a)
{
	struct riscv_regs *r = (struct riscv_regs *)(t->regs) - 1;
	if (n >= 1) r->a0 = a.s;
	if (n >= 2) r->a1 = a.ar0;
	if (n >= 3) r->a2 = a.ar1;
	if (n >= 4) r->a3 = a.ar2;
	if (n >= 5) r->a4 = a.ar3;
	if (n >= 6) r->a5 = a.ar4;
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
	r->sp = (long)t->thread_stack + t->thread_stack_size;
}

void set_stack(struct tcb *t, vm_t s)
{
	/** @todo also set frame pointer on architectures that need it? */
	struct riscv_regs *r = (struct riscv_regs *)(t->regs) - 1;
	r->sp = s;
}

vm_t get_stack(struct tcb *t)
{
	struct riscv_regs *r = (struct riscv_regs *)(t->regs) - 1;
	return r->sp;
}

void copy_regs(struct tcb *d, struct tcb *s)
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
