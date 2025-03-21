/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file proc.c
 * Process handling, might be merged into \ref src/tcb.c.
 */

#include <kmi/elf.h>
#include <kmi/proc.h>
#include <kmi/conf.h>
#include <kmi/debug.h>
#include <kmi/string.h>
#include <kmi/initrd.h>
#include <arch/arch.h>
#include <arch/proc.h>
#include <arch/cpu.h>

#include <libfdt.h>

stat_t prepare_proc(struct tcb *t, vm_t bin, vm_t interp)
{
	vm_t entry = load_elf(t, bin, interp);
	if (!entry)
		return ERR_INVAL;

	t->callback = entry;
	return OK;
}

stat_t init_proc(void *fdt, vm_t *proc_fdt, vm_t *proc_initrd)
{
	init_tcbs();

	/** \todo cleanup or something */
	struct tcb *t = create_proc(NULL);
	if (!t)
		return ERR_OOMEM;

	/* we're the first cpu, so we always have ID 0 */
	t->cpu_id = 0;

	/* force tcb for core */
	tcb_assign(t);

	t->notify_id = t->tid;

	/* init process has all capabilities */
	set_caps(t->caps,
	         CAP_CAPS | CAP_PROC | CAP_SIGNAL | CAP_POWER | CAP_NOTIFY |
	         CAP_SHARED);

	/* we shall try to map the fdt and initrd into the new address space, so
	 * save them here before we switch */
	use_tcb(t);

	set_thread(t);
	set_return(t, t->callback);
	set_stack(t, t->rpc_stack - BASE_PAGE_SIZE);

	stat_t ret = prepare_proc(t, get_init_base(fdt), 0);
	assert(ret == OK);

	/** \todo start one thread per core, with special handling for init in
	 * that each thread starts at the entry point of init? */

	*proc_fdt = map_shared_fixed_uvmem(t,
	                                   (pm_t)fdt, fdt_totalsize(fdt),
	                                   VM_V | VM_R | VM_U);
	assert(*proc_fdt);

	pm_t initrd = (pm_t)__va(get_initrdbase(fdt));
	*proc_initrd = map_shared_fixed_uvmem(t,
	                                      initrd, get_initrdsize(fdt),
	                                      VM_V | VM_R | VM_U);
	assert(proc_initrd);

	info("mapped fdt at %lx\n", *proc_fdt);
	info("mapped initrd at %lx\n", *proc_initrd);
	return OK;
}
