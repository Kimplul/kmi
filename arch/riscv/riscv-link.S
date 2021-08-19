#include <apos/link.lds.h>
#include <vmap.h>

OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
	. = ABSOLUTE(PM_KERN);
	.init ALIGN(4K) : AT(0) {
		*(.init.start);
		*(.init.text);
		*(.init.data);
		. = ALIGN(4K);
	}

	. = ABSOLUTE(VM_KERN);
	.text ALIGN(4K) : AT(SIZEOF(.init)) {
		*(.text.start);
		*(.text.text);
		*(.text.data);
	}
}
