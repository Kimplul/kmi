/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
	/* slight hack, the binary is position independent code but technically
	   not a PIE, so avoid globals and just add the load address to external
	   addresses. */
	__init_start = 0;

	/* objcopy only copies these three sections (as far as I'm aware) into the
	   produced binary, so __init_end should point to the correct location in the
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

	.top : {
		*(.top*)
	}

	__kernel_size = <KERNEL_SIZE>;

	.garbage : {
		*(.note*)
		*(.riscv.attributes)
	}
}
