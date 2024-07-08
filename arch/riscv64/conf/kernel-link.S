/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

OUTPUT_ARCH(riscv)
ENTRY(main)

SECTIONS {
	. = ABSOLUTE(VM_KERNEL);
	__kernel_start = .;
	.text ALIGN(4K) : AT(0) {
		*(.kernel.start);
		*(.text*);
	}

	/* place bss in between different load sections to force objcopy to
	 * output zeroes for it */
	.bss : {
		*(.sbss*)
		*(.bss*)
		*(COMMON)
	}

	.rodata : {
		*(.rodata*)
	}

	.data : {
		*(.data*)
		*(.sdata*)
	}

	__kernel_end = .;

	.garbage : {
		*(.note*)
		*(.riscv.attributes)
	}
}
