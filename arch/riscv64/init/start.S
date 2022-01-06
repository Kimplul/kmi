.section .init.start
.global _start
_start:
li sp, PM_STACK_TOP
call init

.section .text
.global jump_to_kernel
jump_to_kernel:
mv a2, a0 // store kernel addr
mv a0, a1 // move fdt pointer to first argument
jr a2 // jump to kernel
