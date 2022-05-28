/**
 * @file irq.c
 * riscv64 implementation of irq handling.
 */

#include <apos/attrs.h>
#include <apos/debug.h>
#include <arch/irq.h>
#include "csr.h"

/** Defined in arch/riscv64/kernel/entry.S. */
extern void handle_irq();

/** Called from arch/riscv64/kernel/entry.S. */
void handle_sys_irq()
{
}

void init_irq(void *fdt)
{
	UNUSED(fdt);
	csr_write(CSR_STVEC, &handle_irq);

	long s = 0;
	csr_read(CSR_SIE, s);
	info("CSR_SIE: %lx\n", s);
}

/* very simple for now */
void enable_irq()
{
	csr_set(CSR_SSTATUS, CSR_SIE);
}

void disable_irq()
{
	csr_clear(CSR_SSTATUS, CSR_SIE);
}
