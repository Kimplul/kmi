/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2024, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#include <kmi/bkl.h>
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
	assert(!is_rpc(t));

	/* attach to init process */
	struct tcb *init = get_tcb(1);
	reference_thread(init);

	free_stack(t);
	reset_rpc_stack(t);

	id_t old_rid = t->rid;
	t->rid = 1;
	t->pid = 1;
	t->eid = 1;

	clone_uvmem(init->proc.vmem, t->rpc.vmem);
	use_vmem(t->rpc.vmem);
	alloc_stack(t);

	assert(init->callback);
	set_ret4(t, 0, t->tid, SYS_USER_ORPHANED, old_rid);
	set_return(t, init->callback);
	t->callback = init->callback;

	/** @todo release irqs, here or later? Currently leaning towards later
	 * as they don't really take up any resources and the implementation
	 * doesn't make it too easy to search for a specific owner, we can just
	 * discard the IRQ if it turns out that the owner has been orphaned by
	 * that point. */

	bkl_unlock();
	ret_userspace_fast();
	unreachable();
}
