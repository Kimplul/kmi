#ifndef APOS_RISCV_REGS_H
#define APOS_RISCV_REGS_H

/**
 * @file regs.h
 * riscv64 registers, currently only base extension integer regs. Used in
 * _save_context. Used in \ref arch/riscv64/gen/asm-offsets.c to generate a list
 * of offsets usable from assembly.
 *
 * \todo Implement sacing floating point, vector, etc. registers.
 */

/**
 * Structure containing registers that should always be saved.
 * Essentially all base ISA registers.
 */
struct riscv_regs {
	/** Return address. */
	long ra,
	/** Stack pointer. */
	        sp,
	/** Global pointer. */
	        gp,
	/** Thread pointer. */
	        tp,
	/** Temporary 0. */
	        t0,
	/** Temporary 1. */
	        t1,
	/** Temporary 2. */
	        t2,
	/** Caller-save 0. */
	        s0,
	/** Caller-save 1. */
	        s1,
	/** Argument 0. */
	        a0,
	/** Argument 1. */
	        a1,
	/** Argument 2. */
	        a2,
	/** Argument 3. */
	        a3,
	/** Argument 4. */
	        a4,
	/** Argument 5. */
	        a5,
	/** Argument 6. */
	        a6,
	/** Argument 7. */
	        a7,
	/** Caller-save 2. */
	        s2,
	/** Caller-save 3. */
	        s3,
	/** Caller-save 4. */
	        s4,
	/** Caller-save 5. */
	        s5,
	/** Caller-save 6. */
	        s6,
	/** caller-save 7. */
	        s7,
	/** Caller-save 8. */
	        s8,
	/** Caller-save 9. */
	        s9,
	/** Caller-save 10. */
	        s10,
	/** Caller-save 11. */
	        s11,
	/** Temporary 3. */
	        t3,
	/** Temporary 4. */
	        t4,
	/** Temporary 5. */
	        t5,
	/** Temporary 6. */
	        t6;
};

#endif /* APOS_RISCV_REGS_H */
