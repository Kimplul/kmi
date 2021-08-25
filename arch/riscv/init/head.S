.section .init.start
.global _start
.type _start, @function
_start:
	/* stack pointer is whatever? */	
	/* go to proper init */
	j init
