OUTPUT_ARCH(riscv)
ENTRY(main)

SECTIONS {
	. = ABSOLUTE(PM_KERN);
	__kernel_start = .;

	.text : AT(0) {
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
