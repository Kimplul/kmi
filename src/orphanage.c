/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2024, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#include <kmi/assert.h>
#include <kmi/orphanage.h>

#include <arch/proc.h>

/**
 * @file orphanage.c
 *
 * Stuff related to orphaned threads implementation.
 */

bool orphan(struct tcb *t)
{
	return is_set(t->state, TCB_ORPHAN);
}

void orphanize(struct tcb *t)
{
	set_bit(t->state, TCB_ORPHAN);
}

void unorphanize(struct tcb *t)
{
	catastrophic_assert(!is_rpc(t));

	/* attach to init process */
	struct tcb *init = get_tcb(1);
	reference_proc(init);

	t->rid = 1;
	t->pid = 1;
	t->eid = 1;

	free_stack(t);
	reset_rpc_stack(t);

	t->proc = init->proc;
	use_vmem(t->proc.vmem);
	alloc_stack(t);

	clear_bits(t->state, TCB_ORPHAN);

	catastrophic_assert(init->callback);
	set_args3(t, 0, SYS_USER_ORPHANED, t->tid);
	set_return(t, init->callback);
	t->callback = init->callback;

	/** @todo release irqs, here or later? */

	ret_userspace_fast();
	unreachable();
}
