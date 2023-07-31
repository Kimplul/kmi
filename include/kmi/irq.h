/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_IRQ_H
#define KMI_IRQ_H

/**
 * @file irq.h
 * Common IRQ handling stuff.
 */

#include <kmi/tcb.h>
#include <arch/irq.h>

/**
 * Initialize IRQ subsystem.
 *
 * @param fdt Flattened device tree.
 */
void init_irq(void *fdt);

/** Handle irq. */
void handle_irq();

/**
 * Register IRQ with thread \p t.
 *
 * @param t Thread to register IRQ \p id to.
 * @param id IRQ id to register to \p t.
 * @return OK on success, non-zero otherwise.
 */
stat_t register_irq(struct tcb *t, irq_t id);

/**
 * Unregister IRQ with thread \p t.
 *
 * @param t Thread to unregister \p id from.
 * @param id IRQ id to unregister from \p t.
 * @return OK on success, non-zero otherwise.
 */
stat_t unregister_irq(struct tcb *t, irq_t id);

#endif /* KMI_IRQ_H */
