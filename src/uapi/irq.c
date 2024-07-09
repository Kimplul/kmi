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
 * @param id Which IRQ number to register.
 *
 * @return OK on success, non-zero otherwise.
 */
SYSCALL_DEFINE1(irq_req)(struct tcb *t, sys_arg_t id)
{
	if (!has_cap(t->caps, CAP_IRQ))
		return_args1(t, ERR_PERM);

	if (!t->callback || !t->notify_id)
		return_args1(t, ERR_NF);

	return_args1(t, register_irq(t, id));
}

SYSCALL_DEFINE1(free_irq)(struct tcb *t, sys_arg_t id)
{
	if (!has_cap(t->caps, CAP_IRQ))
		return_args1(t, ERR_PERM);

	return_args1(t, unregister_irq(t, id));
}

/**
 * Actual notification handler setter.
 *
 * @param t Current tcb.
 * @param tid Thread whose handler to set. Still not sure if this should just be
 * the current thread, but I guess this might be a bit easier?
 * @param pid Process that is willing to handle notifications for the thread.
 *
 * @return OK on success, non-zero otherwise.
 */
SYSCALL_DEFINE2(set_handler)(struct tcb *t, sys_arg_t tid, sys_arg_t pid)
{
	if (!has_cap(t->caps, CAP_SIGNAL))
		return_args1(t, ERR_PERM);

	struct tcb *r = get_tcb(tid);
	if (!r)
		return_args1(t, ERR_INVAL);

	t->notify_id = pid;
	return_args1(t, OK);
}
