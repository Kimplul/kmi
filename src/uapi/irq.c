/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file irq.c
 * IRQ request syscall handling implementation.
 */

#include <kmi/uapi.h>
#include <kmi/irq.h>

/**
 * Actual IRQ handling request syscall handler.
 *
 * @param t Current tcb.
 *
 * @return OK on success, non-zero otherwise.
 */
SYSCALL_DEFINE1(irq_req)(struct tcb *t, sys_arg_t id)
{
	if (!has_cap(t->caps, CAP_IRQ))
		return_args1(t, ERR_PERM);

	if (!t->callback)
		return_args1(t, ERR_NF);

	return_args1(t, register_irq(t, id));
}

SYSCALL_DEFINE2(req_notification)(struct tcb *t, sys_arg_t tid, sys_arg_t pid)
{
	if (!has_cap(t->caps, CAP_NOTIFICATION))
		return_args1(t, ERR_PERM);

	struct tcb *r = get_tcb(tid);
	if (!r)
		return_args1(t, ERR_INVAL);

	t->notify_id = pid;
	return_args1(t, OK);
}
