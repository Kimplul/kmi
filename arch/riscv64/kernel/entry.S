/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#include "../kernel/gen/asm-offsets.h"
#include "../kernel/csr.h"
#include "asm.h"

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

.macro save_caller
	sr      a0, offsetof_a0(sp)
	sr      a1, offsetof_a1(sp)
	sr      a2, offsetof_a2(sp)
	sr      a3, offsetof_a3(sp)
	sr      a4, offsetof_a4(sp)
	sr      a5, offsetof_a5(sp)
	sr      a6, offsetof_a6(sp)
	sr      a7, offsetof_a7(sp)
	sr      t0, offsetof_t0(sp)
	sr      t1, offsetof_t1(sp)
	sr      t2, offsetof_t2(sp)
	sr      t3, offsetof_t3(sp)
	sr      t4, offsetof_t4(sp)
	sr      t5, offsetof_t5(sp)
	sr      t6, offsetof_t6(sp)
.endm

.macro save_callee
	/* save registers */
	sr      ra, offsetof_ra(sp)
	sr	gp, offsetof_gp(sp)
	sr	tp, offsetof_tp(sp)
	sr      s0, offsetof_s0(sp)
	sr	s1, offsetof_s1(sp)
	sr	s2, offsetof_s2(sp)
	sr	s3, offsetof_s3(sp)
	sr	s4, offsetof_s4(sp)
	sr	s5, offsetof_s5(sp)
	sr	s6, offsetof_s6(sp)
	sr	s7, offsetof_s7(sp)
	sr	s8, offsetof_s8(sp)
	sr	s9, offsetof_s9(sp)
	/* s10 and s11 always saved */
.endm

.global handle_trap
handle_trap:
	csrrw tp, CSR_SSCRATCH, tp
	bnez tp, continue_trap
	/* the trap came from the kernel, should never happen so just panic
	 * and abort or whatever */
	csrw CSR_SSCRATCH, x0
	csrr a0, CSR_SEPC
	csrr a1, CSR_STVAL
	csrr a2, CSR_SCAUSE
	call kernel_panic

continue_trap:
	sr	sp, offsetof_arch(tp)
	lr	sp, offsetof_regs(tp)
	addi	sp, sp, -sizeof_registers
	csrrw	tp, CSR_SSCRATCH, tp

	sr      s10, offsetof_s10(sp)
	sr      s11, offsetof_s11(sp)

	/* get current tcb into tp and set scratch to 0 so we can figure out if
	 * exception occured in kernel or userspace */
	csrrw tp, CSR_SSCRATCH, x0
	lr	s10, offsetof_arch(tp)
	sr	s10, offsetof_sp(sp)

	/* load supervisor cause */
	csrr s10, CSR_SCAUSE
	/* interrupts fall through, exceptions jump */
	/* (leading 1 means the value is interpreted as negative) */
	bge s10, zero, handle_exception

	save_callee
	save_caller

	csrr    t0, CSR_SEPC
	sr	t0, offsetof_exec(tp)

	mv a0, s10
	/* get actual kernel stack into sp */
	mv sp, tp
	call riscv_handle_interrupt

	lr	sp, offsetof_regs(tp)
	addi	sp, sp, -sizeof_registers
	/* set execution continuation */
	lr	s10, offsetof_exec(tp)
	csrw	CSR_SEPC, s10

	j _load_context

handle_exception:
	li s11, EXC_SYSCALL
	/* system exceptions fall through, syscalls and ipis jump */
	/* temp, at some point we want to handle system exceptions as well */
	beq s10, s11, handle_dispatch

	save_callee
	save_caller

	csrr a0, CSR_SEPC
	csrr a1, CSR_STVAL
	csrr a2, CSR_SCAUSE
	sr   a0, offsetof_exec(tp)
	mv sp, tp
	call riscv_handle_exception

	lr	sp, offsetof_regs(tp)
	addi	sp, sp, -sizeof_registers
	/* set execution continuation */
	lr	s10, offsetof_exec(tp)
	csrw	CSR_SEPC, s10

	j	_load_context

handle_dispatch:
	li	s10, SYS_IPC_RESP
	bne	a0, s10, slow_dispatch
	/* we're a sys_ipc_resp */
	lw	s10, offsetof_rid(tp)
	lw	s11, offsetof_pid(tp)
	/* we're in an rpc, so we can skip saving the rest of the registers */
	beq	s10, s11, fast_dispatch

slow_dispatch:
	save_callee

fast_dispatch:
	/* store execution continuation point */
	csrr    s10, CSR_SEPC
	sr	s10, offsetof_exec(tp)
	/* jump to C */
	mv sp, tp
	call	dispatch
	/* if we had a thread switch, load kernel stack of current thread and
	 * restore its context */
	/* get associated kernel stack */
	lr	sp, offsetof_regs(tp)
	addi	sp, sp, -sizeof_registers
	/* set execution continuation */
	lr	s10, offsetof_exec(tp)
	csrw	CSR_SEPC, s10

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

.global ret_userspace_partial
ret_userspace_partial:
	lr	sp, offsetof_regs(tp)
	addi	sp, sp, -sizeof_registers

	lr	s10, offsetof_exec(tp)
	csrw	CSR_SEPC, s10

	csrw CSR_SSCRATCH, tp

	lr	s0, offsetof_s0(sp)
	lr	a0, offsetof_a0(sp)
	lr      a1, offsetof_a1(sp)
	lr      a2, offsetof_a2(sp)
	lr      a3, offsetof_a3(sp)
	lr      a4, offsetof_a4(sp)
	lr      a5, offsetof_a5(sp)
	lr	s0, offsetof_s0(sp)
	lr	s1, offsetof_s1(sp)
	lr	s2, offsetof_s2(sp)
	lr	s3, offsetof_s3(sp)
	lr	s4, offsetof_s4(sp)
	lr	s5, offsetof_s5(sp)
	lr	s6, offsetof_s6(sp)
	lr	s7, offsetof_s7(sp)
	lr	s8, offsetof_s8(sp)
	lr	s9, offsetof_s9(sp)
	lr	s10, offsetof_s10(sp)
	lr	s11, offsetof_s11(sp)
	lr      ra, offsetof_ra(sp)
	lr	gp, offsetof_gp(sp)
	lr	tp, offsetof_tp(sp)

	lr	sp, offsetof_sp(sp)

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
