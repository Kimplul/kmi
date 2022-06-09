/**
 * @file proc.c
 * riscv64 implementation of arch-specific process handling.
 */

#include <apos/tcb.h>
#include <apos/elf.h>
#include "regs.h"
#include "csr.h"

/** \todo actually map fdt into the target address space */
stat_t run_init(struct tcb *t, void *fdt)
{
	csr_write(CSR_SSCRATCH, t);
	__asm__ volatile ("mv sp, %0\n" : : "r" (t->thread_stack_top) : "memory");
	__asm__ volatile ("mv a0, %0\n" : : "r" (fdt) : );
	__asm__ volatile ("sret\n" ::: "memory");
	/* we should never reach this */
	return ERR_ADDR;
}

stat_t set_return(vm_t v)
{
	csr_write(CSR_SEPC, v);
	return OK;
}

stat_t set_ipc(struct tcb *t, id_t pid, id_t tid)
{
	struct riscv_regs *r = (struct riscv_regs *)(--t);
	r->a2 = (long)pid;
	r->a3 = (long)tid;
	return OK;
}

stat_t set_thread(struct tcb *t, vm_t stack)
{
	/* get location of registers in memory */
	/** \todo check alignment, should be fine but just to be sure */
	struct riscv_regs *r = (struct riscv_regs *)(--t);

	/* insert important values into register slots */
	r->sp = (long)stack;
	r->tp = (long)t;

	return OK;
}
