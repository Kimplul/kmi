/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

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
		*(.sbss*) *(.bss*) *(COMMON)
	}

	__kernel_end = .;
}
