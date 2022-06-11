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
.align 4
.global handle_irq
handle_irq:
	csrrw tp, CSR_SSCRATCH, tp
	bnez tp, _save_context
	/* the exception came from the kernel, should probably handle but for
	 * now just spin in place */
	1: j 1b
_save_context:
	/* store user stack pointer and replace it with kernel stack */
	addi	tp, tp,-sizeof_registers
	sr	sp, offsetof_sp(tp)
	mv	sp, tp

	/* move thread pointer back */
	csrrw tp, CSR_SSCRATCH, tp

	/* save registers */
	sr      ra, offsetof_ra(sp)
	sr	gp, offsetof_gp(sp)
	sr	tp, offsetof_tp(sp)
	sr      t0, offsetof_t0(sp)
	sr      t1, offsetof_t1(sp)
	sr      t2, offsetof_t2(sp)
	sr      s0, offsetof_s0(sp)
	sr      a0, offsetof_a0(sp)
	sr      a1, offsetof_a1(sp)
	sr      a2, offsetof_a2(sp)
	sr      a3, offsetof_a3(sp)
	sr      a4, offsetof_a4(sp)
	sr      a5, offsetof_a5(sp)
	sr      a6, offsetof_a6(sp)
	sr      a7, offsetof_a7(sp)
	sr	s1, offsetof_s1(sp)
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

	/* load supervisor cause */
	csrr s4, CSR_SCAUSE
	/* interrupts fall through, exceptions jump */
	bge s4, zero, handle_exception
	/* TODO: handle interrupts */
	j restore_all

handle_exception:
	li t0, EXC_SYSCALL
	/* system exceptions fall through, syscalls jump */
	beq s4, t0, handle_syscall
	j	restore_all

handle_syscall:
	/* add 4 (size of ecall) to EPC to avoid running the same instruction
	 * twice */
	csrr    s0, CSR_SEPC
	addi s0, s0, 4
	csrw	CSR_SEPC, s0
	jal	syscall_dispatch
	/* if we had a thread switch, load kernel stack of current thread and
	 * restore its context */
	/* TODO: is a whole function call necessary? */
	mv	s0, a0
	mv	s1, a1
	/* get associated kernel stack */
	mv	sp, tp
	addi	sp, sp, -sizeof_registers
	/* restore system call result */
	mv	a0, s0
	mv	a1, s1
	j	restore_noreturn

restore_all:
	lr	a0, offsetof_a0(sp)
	lr      a1, offsetof_a1(sp)

restore_noreturn:
	/* restore registers besides possible return value, assume instruction
	 * to return to has already been set (CSR_EPC) */
	csrw CSR_SSCRATCH, tp

	lr      ra, offsetof_ra(sp)
	lr	gp, offsetof_gp(sp)
	lr	tp, offsetof_tp(sp)
	lr      t0, offsetof_t0(sp)
	lr      t1, offsetof_t1(sp)
	lr      t2, offsetof_t2(sp)
	/* a0 and a1 should not be restored when coming from a syscall */
	lr      a2, offsetof_a2(sp)
	lr      a3, offsetof_a3(sp)
	lr      a4, offsetof_a4(sp)
	lr      a5, offsetof_a5(sp)
	lr      a6, offsetof_a6(sp)
	lr	s0, offsetof_s0(sp)
	lr	s1, offsetof_s1(sp)
	lr	s2, offsetof_s2(sp)
	lr	s3, offsetof_s3(sp)
	lr	s4, offsetof_s4(sp)
	lr	s5, offsetof_s4(sp)
	lr	s6, offsetof_s6(sp)
	lr	s7, offsetof_s7(sp)
	lr	s8, offsetof_s8(sp)
	lr	s9, offsetof_s9(sp)
	lr	s10,offsetof_s10(sp)
	lr	s11,offsetof_s11(sp)
	lr      a7, offsetof_a7(sp)
	lr      t3, offsetof_t3(sp)
	lr      t4, offsetof_t4(sp)
	lr      t5, offsetof_t5(sp)
	lr      t6, offsetof_t6(sp)

	/* restore stack pointer */
	lr	sp, offsetof_sp(sp)

	/* assume supervisor for now */
	sret
