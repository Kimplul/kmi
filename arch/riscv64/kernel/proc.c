/**
 * @file proc.c
 * riscv64 implementation of arch-specific process handling.
 */

#include <apos/tcb.h>
#include <apos/elf.h>
#include <regs.h>
#include <csr.h>

stat_t jump_to_userspace(struct tcb *t, int argc, char **argv)
{
	csr_write(CSR_SEPC, t->entry);
	csr_write(CSR_SSCRATCH, t);
	__asm__ volatile ("mv sp, %0\n" : : "r" (t->proc_stack_top) : "memory");
	__asm__ volatile ("sret\n" ::: "memory");
	/* we should never reach this */
	return ERR_ADDR;
}

stat_t return_to_userspace(struct tcb *t)
{
	/* lol */
	return ERR_ADDR;
}

stat_t prepare_thread(struct tcb *t)
{
	/* get location of registers in memory */
	/* TODO: check alignment, should be fine but just to be sure */
	struct riscv_regs *r = (struct riscv_regs *)t;
	r--;

	r->sp = (long)t->proc_stack_top;
	r->tp = (long)t;

	/* set entry point */
	csr_write(CSR_SEPC, t->entry);
	return OK;
}
