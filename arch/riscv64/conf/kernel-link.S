/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

OUTPUT_ARCH(riscv)
ENTRY(main)

SECTIONS {
	/* This is apparently necessary. I *think* it is to tell the linker that
	 * the kernel should be able to run in the bottom half of the address
	 * space, but I don't know for sure and this feels like a fairly major
	 * hack.
	 * @todo look into this further.
	 */
	. = VM_DMAP;
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
		*(.sdata*)
	}

	.bss : {
		*(.sbss*) *(.bss*) *(COMMON)
	}

	__kernel_end = .;
	__kernel_size = __kernel_end - __kernel_start;

	.garbage : {
		*(.note*)
		*(.riscv.attributes)
	}
}
