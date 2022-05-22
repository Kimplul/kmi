#ifndef APOS_RISCV_REGS_H
#define APOS_RISCV_REGS_H

/**
 * @file regs.h
 * riscv64 registers, currently only base extension integer regs. Used to \ref
 * _save_context. Used in \ref arch/riscv64/gen/asm-offsets.c to generate a list
 * of offsets usable from assembly.
 *
 * @todo Implement sacing floating point, vector, etc. registers.
 */

struct riscv_regs {
	long ra, sp, gp, tp, t0, t1, t2, s0, s1, a0, a1, a2, a3, a4, a5, a6, a7,
		s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, t3, t4, t5, t6;
};

#endif /* APOS_RISCV_REGS_H */
