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

/* a0 is fdt, a1 is load_addr, a2 is d, a3 is ram_base, a4 is DMAP */
.global to_kernelspace
to_kernelspace:
lla t0, kernel
add t0, t0, a4
sub t0, t0, a3
add sp, sp, a4
sub sp, sp, a3
jr t0
