/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023 Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#include <kmi/notify.h>
#include <kmi/queue.h>
#include <kmi/ipi.h>

#include <arch/proc.h>
#include <arch/cpu.h>

/** List for keeping track of which threads have an ipi queued. */
static struct queue_head fifo = INIT_QUEUE(fifo);

/**
 * @file ipi.c
 *
 * IPI function implementations.
 */

void send_ipi(struct tcb *t)
{
	/** @todo this should probably have a spinlock guard */
	/** @todo killing a thread should make sure it gets removed from ipi
	 * queue */
	queue_push(&fifo, &t->ipi_queue);
	cpu_send_ipi(t->cpu_id);
}

void unqueue_ipi(struct tcb *t)
{
	queue_del(&t->ipi_queue);
}

void handle_ipi()
{
	struct tcb *t = cur_tcb();
	adjust_ipi(t);

	struct queue_head *q = queue_pop(&fifo);
	if (!q)
		return;

	struct tcb *r = container_of(q, struct tcb, ipi_queue);
	notify(r, 0);
	/* notify didn't take for whatever reason so return whence we came from */
}
