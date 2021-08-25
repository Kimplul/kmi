#include <vmap.h>

OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
	. = ABSOLUTE(PM_KERN);
	.text ALIGN(4K) : AT(0) {
		*(.init.start)
		*(.text);
		. = ALIGN(4K);
	}
}
