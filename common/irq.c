/* SPDX-License-Identifier: copyleft-next-0.3.1 */

/**
 * @file irq.c
 * Common IRQ handling stuff implementations.
 */

#include <kmi/irq.h>
#include <kmi/pmem.h>
#include <kmi/debug.h>
#include <kmi/assert.h>
#include <kmi/string.h>
#include <arch/irq.h>

/** Hold maximum IRQ id supported by system. */
static size_t max_irq;

/** Hold map of IRD ID -> thread id. @todo process id? */
static id_t *irq_map;

void init_irq(void *fdt)
{
	/** @todo check arch max irq and adjust accordingly */
	irq_map = (id_t *)alloc_page(MM_O0);
	memset(irq_map, 0, order_size(MM_O0));
	max_irq = order_size(MM_O0) / sizeof(irq_map[0]);

	setup_irq(fdt);
}

stat_t register_irq(struct tcb *t, irq_t id)
{
	if (id >= max_irq)
		return ERR_INVAL;

	if (irq_map[id])
		return ERR_EXT;

	irq_map[id] = t->tid;
	return activate_irq(id);
}

stat_t unregister_irq(struct tcb *t, irq_t id)
{
	id_t tid = irq_map[id];
	if (tid != t->tid)
		return ERR_PERM;

	irq_map[id] = 0;
	return deactivate_irq(id);
}

void handle_irq()
{
	irq_t id = get_irq();
	hard_assert(id < max_irq, RETURN_VOID);

	id_t tid = irq_map[id];

	if (!tid) {
		bug("unregistered irq: %llu\n", (unsigned long long)id);
		return;
	}

	/** @todo switch to thread */
}
