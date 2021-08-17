#include <vmap.h>

OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
	. = 0; /* no offset */
	.init : {
		*(.init.start);
		*(.init.text);
		*(.init.data);
		. = ALIGN(4K);
	}

	.text ALIGN(VM_KERN) : AT(ADDR(.init) + SIZEOF(.init)) {
		*(.init.start);
		*(.text.start);
		*(.text);
	}
}
