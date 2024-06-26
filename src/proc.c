/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file proc.c
 * Process handling, might be merged into \ref src/tcb.c.
 */

#include <kmi/elf.h>
#include <kmi/proc.h>
#include <kmi/conf.h>
#include <kmi/string.h>
#include <kmi/initrd.h>
#include <arch/arch.h>
#include <arch/proc.h>
#include <arch/cpu.h>

stat_t prepare_proc(struct tcb *t, vm_t bin, vm_t interp)
{
	vm_t entry = load_elf(t, bin, interp);
	if (!entry)
		return ERR_INVAL;

	alloc_stack(t);
	set_thread(t);
	set_return(t, entry);
	return OK;
}

stat_t init_proc(void *fdt)
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

	/* set current tcb */
	use_tcb(t);

	/* init process has all capabilities */
	set_caps(t->caps, 0, CAP_CAPS | CAP_PROC | CAP_CALL | CAP_POWER);

	/* allocate stacks after ELF file to make sure nothing of importance
	 * clashes */
	return prepare_proc(t, get_init_base(fdt), 0);
	/** \todo start one thread per core, with special handling for init in
	 * that each thread starts at the entry point of init? */
}
