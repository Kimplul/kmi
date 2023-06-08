/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#include "../kernel/gen/asm-offsets.h"
#include "../kernel/csr.h"

#if __riscv_xlen == 64
#define sr sd
#define lr ld
#else
#define sr sw
#define lr lw
#endif

/* very much based on linux, but why change it if works, eh? */
.section .text

.option push
/* norelax necessary for LLVM, as relaxed alignment is apparently unimplemented
 * (as of 2023/05/17, clang 14.0.6):
 */
.option norelax
/* align 2, meaning 2^2 = 4 bytes, necessary as CSR_STVEC requires minimum
 * four byte alignment
 */
.align 2
/* we can go back to relaxed mode if we want */
.option pop

.global handle_irq
handle_irq:
	csrrw tp, CSR_SSCRATCH, tp
	bnez tp, _save_context
	/* the exception came from the kernel, should probably handle but for
	 * now just spin in place */
	1: j 1b
_save_context:
	sr	sp, offsetof_tcbd(tp)
	lr	sp, offsetof_regs(tp)
	addi	sp, sp, -sizeof_registers
	csrrw	tp, CSR_SSCRATCH, tp

	/* save registers */
	sr      ra, offsetof_ra(sp)
	sr	gp, offsetof_gp(sp)
	sr	tp, offsetof_tp(sp)
	sr      t0, offsetof_t0(sp)
	sr      t1, offsetof_t1(sp)
	sr      t2, offsetof_t2(sp)
	sr      s0, offsetof_s0(sp)
	sr	s1, offsetof_s1(sp)
	sr      a0, offsetof_a0(sp)
	sr      a1, offsetof_a1(sp)
	sr      a2, offsetof_a2(sp)
	sr      a3, offsetof_a3(sp)
	sr      a4, offsetof_a4(sp)
	sr      a5, offsetof_a5(sp)
	sr      a6, offsetof_a6(sp)
	sr      a7, offsetof_a7(sp)
	sr	s2, offsetof_s2(sp)
	sr	s3, offsetof_s3(sp)
	sr	s4, offsetof_s4(sp)
	sr	s5, offsetof_s5(sp)
	sr	s6, offsetof_s6(sp)
	sr	s7, offsetof_s7(sp)
	sr	s8, offsetof_s8(sp)
	sr	s9, offsetof_s9(sp)
	sr	s10,offsetof_s10(sp)
	sr	s11,offsetof_s11(sp)
	sr      t3, offsetof_t3(sp)
	sr      t4, offsetof_t4(sp)
	sr      t5, offsetof_t5(sp)
	sr      t6, offsetof_t6(sp)

	/* get current tcb into tp and set scratch to 0 so we can figure out if
	 * exception occured in kernel or userspace */
	csrrw tp, CSR_SSCRATCH, x0
	lr	s0, offsetof_tcbd(tp)
	sr	s0, offsetof_sp(sp)
	addi	sp, tp, -sizeof_registers

	/* load supervisor cause */
	csrr s4, CSR_SCAUSE
	/* interrupts fall through, exceptions jump */
	bge s4, zero, handle_exception
	/* TODO: handle interrupts */
	j _load_context

handle_exception:
	li t0, EXC_SYSCALL
	/* system exceptions fall through, syscalls and ipis jump */
	/* temp, at some point we want to handle system exceptions as well */
	beq s4, t0, handle_dispatch
	j	_load_context

handle_dispatch:
	/* store execution continuation point */
	csrr    s0, CSR_SEPC
	sr	s0, offsetof_exec(tp)
	/* jump to C */
	jal	dispatch
	/* if we had a thread switch, load kernel stack of current thread and
	 * restore its context */
	/* get associated kernel stack */
	lr	sp, offsetof_regs(tp)
	addi	sp, sp, -sizeof_registers
	/* set execution continuation */
	lr	s0, offsetof_exec(tp)
	csrw	CSR_SEPC, s0

_load_context:
	/* restore registers besides possible return value, assume instruction
	 * to return to has already been set (CSR_EPC) */
	csrw CSR_SSCRATCH, tp

	lr      ra, offsetof_ra(sp)
	lr	gp, offsetof_gp(sp)
	lr	tp, offsetof_tp(sp)
	lr      t0, offsetof_t0(sp)
	lr      t1, offsetof_t1(sp)
	lr      t2, offsetof_t2(sp)
	lr	s0, offsetof_s0(sp)
	lr	s1, offsetof_s1(sp)
	lr	a0, offsetof_a0(sp)
	lr      a1, offsetof_a1(sp)
	lr      a2, offsetof_a2(sp)
	lr      a3, offsetof_a3(sp)
	lr      a4, offsetof_a4(sp)
	lr      a5, offsetof_a5(sp)
	lr      a6, offsetof_a6(sp)
	lr      a7, offsetof_a7(sp)
	lr	s2, offsetof_s2(sp)
	lr	s3, offsetof_s3(sp)
	lr	s4, offsetof_s4(sp)
	lr	s5, offsetof_s5(sp)
	lr	s6, offsetof_s6(sp)
	lr	s7, offsetof_s7(sp)
	lr	s8, offsetof_s8(sp)
	lr	s9, offsetof_s9(sp)
	lr	s10,offsetof_s10(sp)
	lr	s11,offsetof_s11(sp)
	lr      t3, offsetof_t3(sp)
	lr      t4, offsetof_t4(sp)
	lr      t5, offsetof_t5(sp)
	lr      t6, offsetof_t6(sp)

	/* restore stack pointer */
	lr	sp, offsetof_sp(sp)

	/* assume supervisor for now */
	sret

.global ret_userspace_fast
ret_userspace_fast:
	lr	sp, offsetof_regs(tp)
	addi	sp, sp, -sizeof_registers

	lr	s0, offsetof_exec(tp)
	csrw	CSR_SEPC, s0

	csrw CSR_SSCRATCH, tp

	lr	a0, offsetof_a0(sp)
	lr      a1, offsetof_a1(sp)
	lr      a2, offsetof_a2(sp)
	lr      a3, offsetof_a3(sp)
	lr      a4, offsetof_a4(sp)
	lr      a5, offsetof_a5(sp)

	lr	sp, offsetof_sp(sp)

	sret
