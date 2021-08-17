.section .init.start
.global _start
.type _start, @function
_start:
	/* store kernel load address */
	auipc a4, 0
	/* stack grows downward */
	mv sp, a4
	/* go to proper init */
	j init
