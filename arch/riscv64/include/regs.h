#ifndef APOS_RISCV_REGS_H
#define APOS_RISCV_REGS_H

struct riscv_regs {
	long ra, sp, gp, tp, t0, t1, t2, s0, s1, a0, a1, a2, a3, a4, a5, a6, a7,
		s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, t3, t4, t5, t6;
};

#endif /* APOS_RISCV_REGS_H */
