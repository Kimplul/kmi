/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file main.c
 * Entry point for actual kernel setup.
 */

#include <kmi/mem_nodes.h>
#include <kmi/timer.h>
#include <kmi/attrs.h>
#include <kmi/proc.h>
#include <kmi/debug.h>
#include <kmi/vmem.h>
#include <arch/arch.h>
#include <arch/proc.h>
#include <arch/irq.h>
#include <libfdt.h>

/**
 * Boot entry of kernel actual.
 *
 * Sets up all kernel subsystems and jumps into \c init program, does not
 * return.
 *
 * @param fdt Global FDT pointer.
 * @return Should not.
 */
void __main main(void *fdt)
{
	/* dbg uses direct mapping at this point */
	init_dbg(fdt);
	setup_dmap_dbg();
	dbg_fdt(fdt);

	setup_arch(fdt);

	init_pmem(fdt);
	/* setup temporary virtual memory */
	struct vmem *b = init_vmem(fdt);

	/* start up debugging in kernel IO */
	setup_io_dbg(b);

	init_irq(fdt);
	init_timer(fdt);
	init_proc(fdt);
	/* free temporary virtual memory now that we're in the init process
	 * space */
	destroy_vmem(b);

	/* start running init program */
	run_init(cur_tcb(), fdt);
}
