#if GENERIC_UBOOT
# define MAIN main_go
#else
# define MAIN main
#endif

.section .kernel.start
.global _start
_start:
/* get load address */
auipc a2, 0

/* load initial stack */
lla sp, riscv_init_stack
li t0, 4096
add sp, sp, t0

/* set thread pointer to zero so we don't accidentally try to use a tcb */
li tp, 0

/* call main, whichever one is relevant */
call MAIN
ret

.section .text

/* a0 is fdt, a1 is load_addr, a2 is d, a3 is ram_base */
.global to_kernelspace
to_kernelspace:
li t1, VM_DMAP
add sp, sp, t1
lui t0, %hi(kernel)
addi t0, t0, %lo(kernel)
jr t0
