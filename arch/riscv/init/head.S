.section .init.start
.global _start
.type _start, @function
_start:
	li sp, PM_STACK_TOP
	/* go to proper init */
	j init
