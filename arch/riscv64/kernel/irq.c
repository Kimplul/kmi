/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file irq.c
 * riscv64 implementation of irq handling.
 */

#include <kmi/attrs.h>
#include <kmi/debug.h>
#include <kmi/timer.h>
#include <kmi/ipi.h>
#include <kmi/irq.h>
#include "csr.h"

/** Defined in arch/riscv64/kernel/entry.S. */
extern void handle_trap();

void setup_irq(void *fdt)
{
	UNUSED(fdt);
	csr_write(CSR_STVEC, &handle_trap);

	long s = 0;
	csr_read(CSR_SIE, s);
	info("CSR_SIE: %lx\n", s);
}

stat_t activate_irq(irq_t id)
{
	/** @todo implement plic */
	return OK;
}

stat_t deactivate_irq(irq_t id)
{
	/** @todo implement plic */
	return OK;
}

/* very simple for now */
void enable_irqs()
{
	/* should be supervisor external interrupt */
	csr_set(CSR_SIE, 1 << 9);
}

void disable_irqs()
{
	csr_clear(CSR_SIE, 1 << 9);
}

irq_t get_irq()
{
	/** @todo implement */
	return 0;
}

/**
 * Prints out a bug message about unknown interrupt cause.
 *
 * @param id ID of interrupt.
 */
static void riscv_unknown_interrupt(long id)
{
	bug("unknown interrupt: %llu\n", (unsigned long long)id);
}

/**
 * Handle RISCV interrupt.
 * Branches out to correct interrupt handler.
 *
 * @param id ID of interrupt, with interrupt bit still set.
 */
void riscv_handle_interrupt(unsigned long id)
{
	/* clear leading one */
	id = (id << 1) >> 1;

	switch (id) {
	/* I think */
	case 1: handle_ipi(); break;
	case 5: handle_timer(); break;
	case 9: handle_irq(); break;
	default: riscv_unknown_interrupt(id); break;
	}
}
