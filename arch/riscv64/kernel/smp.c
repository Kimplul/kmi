/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file smp.c
 * riscv multicore bringup implementation.
 */

#include <kmi/debug.h>
#include <kmi/bkl.h>
#include <kmi/tcb.h>
#include <libfdt.h>

#include <arch/proc.h>
#include <arch/arch.h>
#include <arch/smp.h>
#include "arch.h"
#include "sbi.h"

/**
 * Array of stacks for bringing up harts.
 * Note not static since we want to access it from core_bringup.S.
 */
void *smp_init_stacks[MAX_CPUS];

/**
 * Check if FDT node is a cpu and that its status is 'okay'.
 * Helper for smp_bringup(), cpu nodes that are marked 'okay' in the FDT should
 * be brought up.
 *
 * @param fdt Flattened device tree.
 * @param node Node to check.
 * @return true if node is a cpu node and it should be brought up, false
 * otherwise.
 */
static bool riscv_cpu_okay(void *fdt, int node)
{
	const char *s = fdt_getprop(fdt, node, "device_type", NULL);
	if (!s || strcmp(s, "cpu") != 0)
		return false;

	s = fdt_getprop(fdt, node, "status", NULL);
	if (!s)
		return false;

	if (strcmp(s, "okay") == 0)
		return true;

	return false;
}

/** Counter for how many cores system has. */
static size_t cpus = 1;

/* called from main to start other cores */
void smp_bringup(struct vmem *b, void *fdt)
{
	/* defined in core_bringup.S */
	extern void riscv_bringup(void);

	/* mark first hart available */
	set_hartid(0, -1);

	/* assume we're in default Sv mode, in the future this will have to be
	 * fixed if we start implementing Sv48 etc. */
	pm_t satp = branch_to_satp(b, DEFAULT_Sv_MODE);
	int cpu_offset = fdt_path_offset(fdt, "/cpus");
	struct cell_info ci = get_cellinfo(fdt, cpu_offset);

	/* almost directly lifted from netbsd */
	int node;
	fdt_for_each_subnode(node, fdt, cpu_offset) {
		if (!riscv_cpu_okay(fdt, node))
			continue;

		const void *reg = fdt_getprop(fdt, node, "reg", NULL);
		id_t hartid = fdt_load_reg_addr(ci, reg, 0);

		struct sbiret r = sbi_hart_status(hartid);
		if (r.error) {
			warn("failed getting hart %ld status: %ld\n",
			     (long)hartid,
			     (long)r.error);
			continue;
		}

		if (r.value == SBI_HART_STARTED) {
			/* there should ever only be one started hart */
			assert(cpuid_to_hartid(0) == -1);
			set_hartid(0, hartid);
			continue;
		}

		/** @todo should check that cpus doesn't go over MAX_CPUS */
		set_hartid(cpus++, hartid);

		/** @todo try to remember to free these as well */
		smp_init_stacks[hartid] = (void *)alloc_page(BASE_PAGE) +
		                          BASE_PAGE_SIZE;

		/* fixup physical address of bringup */
		pm_t bringup = (pm_t)riscv_bringup
		               - VM_KERNEL
		               + get_load_addr();

		r = sbi_hart_start(hartid, bringup, satp);

		if (r.error) {
			warn("failed bringing up hart %ld: %ld\n",
			     (long)hartid,
			     (long)r.error);
			continue;
		}
	}
}

/**
 * Called from secondary_bringup.S to finish bringing up core we're running on.
 *
 * @param hartid Hart that's being brought up.
 */
__noreturn void core_bringup(long hartid)
{
	bkl_lock();
	/* assume smp_bringup assigned our cpuid correctly */
	id_t cpuid = hartid_to_cpuid(hartid);

	/* output is somewhat messed up due to no synchronisation but I guess
	 * that's fine for now */
	info("core %ld online\n", (long)cpuid);

	/* realistically stuff after this point could probably be placed
	 * somewhere in common/ */

	/* add us as a thread to init program that cpu 0 is hopefully running by
	 * now */
	struct tcb *init = get_tcb(1);
	assert(init);

	struct tcb *t = create_thread(init);
	assert(t);

	/* init is special in that all threads jump to the entrypoint of the
	 * program */
	t->exec = init->exec;
	t->cpu_id = cpuid;
	/** @todo this is pretty hacky, should really be a separate function? */
	setup_irq(NULL);
	setup_arch(NULL);
	tcb_assign(t);
	use_tcb(t);

	/* now that we're in our own virtual address space, set stack etc. */
	set_thread(t);

	info("core %ld releasing BKL\n", (long)cpuid);
	run_init(t, NULL, NULL);
	unreachable();
}
