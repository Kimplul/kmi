#include <csr.h>

#if __riscv_xlen == 64
#define sr sd
#define lr ld
#else
#define sr sw
#define lr lw
#endif

.section .text
.global handle_irq
handle_irq:
	addi    sp,sp,-144
	sr      ra,136(sp)
	sr      t0,128(sp)
	sr      t1,120(sp)
	sr      t2,112(sp)
	sr      s0,104(sp)
	sr      a0,96(sp)
	sr      a1,88(sp)
	sr      a2,80(sp)
	sr      a3,72(sp)
	sr      a4,64(sp)
	sr      a5,56(sp)
	sr      a6,48(sp)
	sr      a7,40(sp)
	sr      t3,32(sp)
	sr      t4,24(sp)
	sr      t5,16(sp)
	sr      t6,8(sp)
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
	lr      a0,96(sp)

restore_noreturn:
	lr      ra,136(sp)
	lr      t0,128(sp)
	lr      t1,120(sp)
	lr      t2,112(sp)
	lr      s0,104(sp)
	lr      a1,88(sp)
	lr      a2,80(sp)
	lr      a3,72(sp)
	lr      a4,64(sp)
	lr      a5,56(sp)
	lr      a6,48(sp)
	lr      a7,40(sp)
	lr      t3,32(sp)
	lr      t4,24(sp)
	lr      t5,16(sp)
	lr      t6,8(sp)
	addi    sp,sp,144
	/* assume supervisor for now */
	sret
