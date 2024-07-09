/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file asm-offsets.c
 * riscv64 file used to generate offsets to register slots in stack. Used by
 * _save_context.
 */

#include <kmi/syscalls.h>
#include <kmi/utils.h>
#include <kmi/uapi.h>
#include "../kernel/regs.h"

/**
 * Insert assembly comment with calculated value.
 * Largely based on linux
 *
 * @param sym Name of symbol.
 * @param val Value of symbol.
 */
#define DEFINE(sym, val) \
	__asm__ volatile ("\n#-> " #sym " %0 " #val "\n" : : "i" (val))

/**
 * Insert offset of member in structure.
 *
 * @param m Member.
 * @param s Structure.
 */
#define OFFSETOF(m, s) DEFINE(offsetof_##m, offsetof(s, m))

/**
 * Insert size of structure.
 *
 * @param n Name of structure.
 * @param s Structure.
 */
#define SIZEOF(n, s) DEFINE(sizeof_##n, sizeof(s))

/**
 * Insert enum.
 *
 * @param e Name of enum value.
 */
#define ENUM(e) DEFINE(e, e)

/**
 * Generate assembly with calculated offsets and sizes.
 * See arch/riscv64/gen/source.mk for how the assembly is used.
 */
void asm_offsets()
{
	OFFSETOF(ra, struct riscv_regs);
	OFFSETOF(sp, struct riscv_regs);
	OFFSETOF(tp, struct riscv_regs);
	OFFSETOF(gp, struct riscv_regs);
	OFFSETOF(t0, struct riscv_regs);
	OFFSETOF(t1, struct riscv_regs);
	OFFSETOF(t2, struct riscv_regs);
	OFFSETOF(s0, struct riscv_regs);
	OFFSETOF(s1, struct riscv_regs);
	OFFSETOF(a6, struct riscv_regs);
	OFFSETOF(a7, struct riscv_regs);
	OFFSETOF(s2, struct riscv_regs);
	OFFSETOF(s3, struct riscv_regs);
	OFFSETOF(s4, struct riscv_regs);
	OFFSETOF(s5, struct riscv_regs);
	OFFSETOF(s6, struct riscv_regs);
	OFFSETOF(s7, struct riscv_regs);
	OFFSETOF(s8, struct riscv_regs);
	OFFSETOF(s9, struct riscv_regs);
	OFFSETOF(s10, struct riscv_regs);
	OFFSETOF(s11, struct riscv_regs);
	OFFSETOF(t3, struct riscv_regs);
	OFFSETOF(t4, struct riscv_regs);
	OFFSETOF(t5, struct riscv_regs);
	OFFSETOF(t6, struct riscv_regs);
	OFFSETOF(a0, struct riscv_regs);
	OFFSETOF(a1, struct riscv_regs);
	OFFSETOF(a2, struct riscv_regs);
	OFFSETOF(a3, struct riscv_regs);
	OFFSETOF(a4, struct riscv_regs);
	OFFSETOF(a5, struct riscv_regs);
	SIZEOF(registers, struct riscv_regs);

	SIZEOF(sys_ret, struct sys_ret);

	OFFSETOF(exec, struct tcb);
	OFFSETOF(regs, struct tcb);
	OFFSETOF(rid, struct tcb);
	OFFSETOF(pid, struct tcb);
	/* At the moment tcbd is just a single register slot, so this works, but
	 * if it's expanded in the future I'll need to figure out a way to
	 * target specific substructure members. */
	OFFSETOF(arch, struct tcb);
	SIZEOF(tcb, struct tcb);

	ENUM(SYS_IPC_RESP);
}
