.section .init.start
.global _start
_start:
li sp, PM_STACK_TOP
jal init
