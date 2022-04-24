#include <csr.h>

.section .text
.global handle_irq
handle_irq:
	addi    sp,sp,-144
	sd      ra,136(sp)
	sd      t0,128(sp)
	sd      t1,120(sp)
	sd      t2,112(sp)
	sd      s0,104(sp)
	sd      a0,96(sp)
	sd      a1,88(sp)
	sd      a2,80(sp)
	sd      a3,72(sp)
	sd      a4,64(sp)
	sd      a5,56(sp)
	sd      a6,48(sp)
	sd      a7,40(sp)
	sd      t3,32(sp)
	sd      t4,24(sp)
	sd      t5,16(sp)
	sd      t6,8(sp)
	addi    s0,sp,144
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
	jal	syscall_dispatch
	j	restore_noreturn

restore_all:
	ld      a0,96(sp)

restore_noreturn:
	ld      ra,136(sp)
	ld      t0,128(sp)
	ld      t1,120(sp)
	ld      t2,112(sp)
	ld      s0,104(sp)
	ld      a1,88(sp)
	ld      a2,80(sp)
	ld      a3,72(sp)
	ld      a4,64(sp)
	ld      a5,56(sp)
	ld      a6,48(sp)
	ld      a7,40(sp)
	ld      t3,32(sp)
	ld      t4,24(sp)
	ld      t5,16(sp)
	ld      t6,8(sp)
	addi    sp,sp,144
	/* assume supervisor for now */
	sret
