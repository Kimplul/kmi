/* SPDX-License-Identifier: copyleft-next-0.3.1 */
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
#include <kmi/irq.h>
#include <arch/arch.h>
#include <arch/proc.h>
#include <arch/smp.h>
#include <libfdt.h>

/**
 * Boot entry of kernel actual.
 *
 * Sets up all kernel subsystems and jumps into \c init program, does not
 * return.
 *
 * @param fdt Global FDT pointer in physical memory.
 * @param ram_base RAM base.
 * @return Should not.
 */
void __main main(void *fdt, uintptr_t ram_base)
{
	set_ram_base(ram_base);

	/* convert physical address to virtual address */
	fdt = __va(fdt);

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

	/* try to bring up other cores on system */
	smp_bringup(b, fdt);

	/* start running init program */
	run_init(cur_tcb(), fdt);
}
