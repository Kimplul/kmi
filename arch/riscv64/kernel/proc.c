/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file proc.c
 * riscv64 implementation of arch-specific process handling.
 */

#include <apos/tcb.h>
#include <apos/elf.h>
#include "regs.h"
#include "csr.h"

stat_t run_init(struct tcb *t, void *fdt)
{
	/** \todo actually map fdt into the target address space */
	csr_write(CSR_SSCRATCH, t);
	csr_write(CSR_SEPC, t->exec);
	__asm__ volatile ("mv sp, %0\n" : : "r" (t->thread_stack_top) : "memory");
	__asm__ volatile ("mv a0, %0\n" : : "r" (fdt) : );
	__asm__ volatile ("sret\n" ::: "memory");
	/* we should never reach this */
	return ERR_ADDR;
}

stat_t set_ipc(struct tcb *t, id_t pid, id_t tid)
{
	struct riscv_regs *r = (struct riscv_regs *)(--t);
	r->a2 = (long)pid;
	r->a3 = (long)tid;
	return OK;
}

stat_t set_thread(struct tcb *t)
{
	/* get location of registers in memory */
	/** \todo check alignment, should be fine but just to be sure */
	struct riscv_regs *r = (struct riscv_regs *)(--t);

	/* insert important values into register slots */
	r->sp = (long)t->thread_stack_top;
	r->tp = (long)t->thread_storage;

	return OK;
}
