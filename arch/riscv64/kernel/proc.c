/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file proc.c
 * riscv64 implementation of arch-specific process handling.
 */

#include <kmi/tcb.h>
#include <kmi/elf.h>
#include <kmi/bkl.h>
#include <kmi/debug.h>
#include <kmi/string.h>

#include <arch/proc.h>

#include "regs.h"
#include "pte.h"
#include "csr.h"

/** Assembly implementation for actually jumping to the init process, defined in
 * start.S. Quite a few constants that could be implemented in assembly as well
 * but this is maybe a bit more convenient.
 *
 * @param pid Should always be 0 to indicate that the kernel is the originator.
 * @param tid Thread ID of the current thread.
 * @param code Should always be SYS_USER_SPAWNED to indicate that a new core has
 * come online.
 * @param fdt Address of flattened device tree within userspace memory.
 * @param initrd Ditto for initial ramdisk.
 * @param proc Process ID, should be constant 1.
 * @param stack_top Stack address.
 */
__noreturn void riscv_run_init(sys_arg_t pid,
                               sys_arg_t tid,
                               sys_arg_t code,
                               sys_arg_t fdt,
                               sys_arg_t initrd,
                               sys_arg_t proc, /* not strictly speaking necessary but eh */
                               sys_arg_t stack_top);

void run_init(struct tcb *t, vm_t fdt, vm_t initrd)
{
	csr_write(CSR_SSCRATCH, t);
	csr_write(CSR_SEPC, t->callback);

	/* reference main virtual memory */
	struct tcb *r = get_rproc(t);
	clone_uvmem(r->proc.vmem, t->rpc.vmem);
	flush_tlb_all();

	/* lore, should probably be codified somewhere */
	vm_t stack_top = get_stack(t);
	info("jumping to %lx with sp = %lx\n", (long)t->callback, stack_top);

	bkl_unlock();
	riscv_run_init(0, t->tid, SYS_USER_SPAWNED, fdt, initrd, 1, stack_top);
	/* we should never reach this */
	unreachable();
}

/**
 * Safely calculate where t->regs actually is, so we can access the rpc stack
 * even without being in the same address space. Allows us to avoid doing some
 * cache flushes in src/uapi/proc.c.
 *
 * @param t Thread whose registers we want to access.
 * @return The physical address of the current registers.
 */
static __inline pm_t __physical_regs(struct tcb *t)
{
	pm_t offset = (vm_t)t->regs - RPC_STACK_BASE;
	return t->arch.rpc_page + offset;
}

void set_ret_fast(struct tcb *t, struct sys_ret a)
{
	struct riscv_regs *r = (struct riscv_regs *)(t->regs) - 1;
	r->a0 = a.s;
	r->a1 = a.id;
	r->a2 = a.a0;
	r->a3 = a.a1;
	r->a4 = a.a2;
	r->a5 = a.a3;
}

void set_ret(struct tcb *t, size_t n, struct sys_ret a)
{
	pm_t regs = __physical_regs(t);
	struct riscv_regs *r = (struct riscv_regs *)(regs) - 1;
	if (n >= 1) r->a0 = a.s;
	if (n >= 2) r->a1 = a.id;
	if (n >= 3) r->a2 = a.a0;
	if (n >= 4) r->a3 = a.a1;
	if (n >= 5) r->a4 = a.a2;
	if (n >= 6) r->a5 = a.a3;
}

struct sys_ret get_ret(struct tcb *t)
{
	pm_t regs = __physical_regs(t);
	struct riscv_regs *r = (struct riscv_regs *)(regs) - 1;
	return SYS_RET6(r->a0, r->a1, r->a2, r->a3, r->a4, r->a5);
}

void set_thread(struct tcb *t)
{
	pm_t regs = __physical_regs(t);
	/* get location of registers in memory */
	/** \todo check alignment, should be fine but just to be sure */
	struct riscv_regs *r = (struct riscv_regs *)(regs) - 1;

	/* insert important values into register slots */
	r->sp = (long)t->rpc_stack - BASE_PAGE_SIZE;
}

void set_stack(struct tcb *t, vm_t s)
{
	/** @todo also set frame pointer on architectures that need it? */
	pm_t regs = __physical_regs(t);
	struct riscv_regs *r = (struct riscv_regs *)(regs) - 1;
	r->sp = s;
}

void set_stack_fast(struct tcb *t, vm_t s)
{
	struct riscv_regs *r = (struct riscv_regs *)(t->regs) - 1;
	r->sp = s;
}

vm_t get_stack(struct tcb *t)
{
	pm_t regs = __physical_regs(t);
	struct riscv_regs *r = (struct riscv_regs *)(regs) - 1;
	return r->sp;
}

void copy_regs(struct tcb *d, struct tcb *s)
{
	pm_t rgs = __physical_regs(s);
	pm_t rgd = __physical_regs(d);

	struct riscv_regs *rs = (struct riscv_regs *)(rgs) - 1;
	struct riscv_regs *rd = (struct riscv_regs *)(rgd) - 1;
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
