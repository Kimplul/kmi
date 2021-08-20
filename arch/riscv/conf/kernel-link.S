#include <apos/link.lds.h>
#include <vmap.h>

OUTPUT_ARCH(riscv)
ENTRY(main)

SECTIONS {
	. = ABSOLUTE(VM_KERN);
	.text ALIGN(4K) : AT(0) {
		*(.text.start);
		*(.text);
		. = ALIGN(4K);
	}
}
