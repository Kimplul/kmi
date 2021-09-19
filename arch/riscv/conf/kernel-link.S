#include <vmap.h>

OUTPUT_ARCH(riscv)
ENTRY(main)

SECTIONS {
	. = ABSOLUTE(VM_KERN);
	__kernel_start = .;

	.text ALIGN(4K) : AT(0) {
		*(.kernel.start);
		*(.text*);
	}

	.rodata : {
		*(.rodata*)
	}

	.data : {
		*(.data*)
	}

	.bss : {
		*(.bss*) *(COMMON)
	}

	__kernel_end = .;
}
