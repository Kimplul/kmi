/* SPDX-License-Identifier: copyleft-next-0.3.1 */

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
 * @param id IRQ id to request to handle.
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
