OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
	. = ABSOLUTE(PM_KERN_BASE);
	__init_start = .;

	/* objcopy only copies these three sections (as far as I'm aware) into the
	   produces binary, so __init_end should point to the correct location in the
	   final binary
	 */
	.text ALIGN(4K) : AT(0) {
		*(.init.start)
		*(.text*);
	}

	.rodata : {
		*(.rodata*)
	}

	.data : {
		*(.data*)
	}

	.bss : {
		*(.sbss*) *(.bss*) *(COMMON)
	}

	__init_end = . ;
	__kernel_size = <KERNEL_SIZE>;
}
