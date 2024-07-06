/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file main.c
 * Entry point for actual kernel setup.
 */

#include <kmi/regions.h>
#include <kmi/initrd.h>
#include <kmi/timer.h>
#include <kmi/attrs.h>
#include <kmi/proc.h>
#include <kmi/debug.h>
#include <kmi/vmem.h>
#include <kmi/irq.h>
#include <kmi/bkl.h>
#include <arch/arch.h>
#include <arch/proc.h>
#include <arch/smp.h>
#include <libfdt.h>

/**
 * @param fdt Flattened devicetree.
 * @return Base address of RAM. Does not take into account disjoint RAM.
 */
static pm_t __fdt_ram_base(void *fdt)
{
	int mem_offset = fdt_path_offset(fdt, "/memory");
	const void *mem_reg = fdt_getprop(fdt, mem_offset, "reg", NULL);

	struct cell_info ci = get_cellinfo(fdt, mem_offset);
	return (pm_t)fdt_load_reg_addr(ci, mem_reg, 0);
}

/**
 * @param fdt Flattened devicetree.
 * @return RAM size. Does not take into account disjoint RAM.
 */
static pm_t __fdt_ram_size(void *fdt)
{
	int mem_offset = fdt_path_offset(fdt, "/memory");
	const void *mem_reg = fdt_getprop(fdt, mem_offset, "reg", NULL);

	/* here we actually want the root offset because /memory itself doesn't
	 * have children, I guess? */
	struct cell_info ci = get_cellinfo(fdt, fdt_path_offset(fdt, "/"));
	return (pm_t)fdt_load_reg_size(ci, mem_reg, 0);
}

/**
 * 'Actual' kernel, runs in kernelspace and does the heavy lifting during
 * booting.
 *
 * @param fdt Flattened device tree.
 * @param load_addr Where in memory the kernel blob was loaded to.
 * @param d Direct mapping to use during booting.
 */
__noreturn void kernel(void *fdt, uintptr_t load_addr, struct vmem *d)
{
	/* we should be in kernelspace, so use the virtual address of our FDT. */
	fdt = __va(fdt);

	/* dbg uses direct mapping at this point */
	init_dbg(fdt);
	/* start up debugging in kernel IO */
	setup_io_dbg(d);

	//dbg_fdt(fdt);

	setup_arch(fdt);

	init_pmem(fdt, load_addr);

	init_irq(fdt);
	init_timer(fdt);

	vm_t proc_fdt = 0, proc_initrd = 0;
	init_proc(fdt, &proc_fdt, &proc_initrd);

	/* lock kernel since we're about to start other threads as well */
	bkl_lock();
	/* try to bring up other cores on system */
	smp_bringup(d, fdt);

	/* start running init program */
	run_init(cur_tcb(), proc_fdt, proc_initrd);
	unreachable();
}

/**
 * Boot entry of kernel actual.
 *
 * Sets up all kernel subsystems and jumps into \c init program, does not
 * return.
 *
 * @param hart Hart ID. Technically unused, but makes the signature more fitting
 * for booting with different systems, like OpenSBI and u-boot.
 * @param fdt Global FDT pointer in physical memory.
 * @param load_addr To which (physical) address in RAM kernel was loaded to.
 */
__noreturn void main(unsigned long hart, void *fdt, uintptr_t load_addr)
{
	/* we have our own ways to get the current hart when we need it, but we
	 * have to get the function signature right */
	(void)hart;

	/** @todo some kind of lottery? */

	pm_t ram_base = __fdt_ram_base(fdt);
	pm_t ram_size = __fdt_ram_size(fdt);
	set_ram_base(ram_base);
	set_ram_size(ram_size);
	set_load_addr(load_addr);

	init_mem(fdt);

	struct vmem *d = direct_mapping();

	to_kernelspace(fdt, load_addr, d, ram_base, VM_DMAP);
	unreachable();
}

#if GENERIC_UBOOT
void main_go(size_t argc, char *argv[], uintptr_t load_addr)
{
	if (argc != 2)
		return;

	void *fdt = (void *)strtouintptr(argv[1]);

	main(0, fdt, load_addr);
}
#endif
