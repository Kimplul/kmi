/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023 Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#include <kmi/ipi.h>
#include <arch/cpu.h>

#include <arch/proc.h>

/**
 * @file ipi.c
 *
 * IPI function implementations.
 */

bool clear_ipi(struct tcb *t)
{
	bool r = t->ipi;
	t->ipi = false;
	return r;
}

void send_ipi(struct tcb *t)
{
	t->ipi = true;
	cpu_send_ipi(t->cpu_id);
}

struct sys_ret handle_ipi(struct tcb *t)
{
	adjust_ipi(t);

	/** @todo use rpc stack */
	set_return(t, t->callback);
	return get_args(t);
}
