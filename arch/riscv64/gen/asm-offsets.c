/**
 * @file asm-offsets.c
 * riscv64 file used to generate offsets to register slots in stack. See \ref
 * _save_context.
 */

#include <apos/utils.h>
#include "../kernel/regs.h"

/* largely based on linux */
#define DEFINE(sym, val)                                                       \
	__asm__ volatile ("\n#-> " #sym " %0 " #val "\n" : : "i" (val))
#define OFFSETOF(m, s) DEFINE(offsetof_##m, offsetof(s, m))
#define SIZEOF(n, s)   DEFINE(sizeof_##n, sizeof(s))

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
	OFFSETOF(a0, struct riscv_regs);
	OFFSETOF(a1, struct riscv_regs);
	OFFSETOF(a2, struct riscv_regs);
	OFFSETOF(a3, struct riscv_regs);
	OFFSETOF(a4, struct riscv_regs);
	OFFSETOF(a5, struct riscv_regs);
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
	SIZEOF(registers, struct riscv_regs);
}
