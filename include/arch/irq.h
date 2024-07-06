/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_ARCH_IRQ_H
#define KMI_ARCH_IRQ_H

/** Type for IRQ id. */
typedef uint_fast32_t irq_t;

/**
 * @file irq.h
 * Arch-specific interrupt handling, generally implemented in
 * arch/whatever/kernel/irq.c
 *
 * \todo These should probably also be stat_t...
 */

#if defined(__riscv)
# if __riscv_xlen == 64
#include "../../arch/riscv64/include/irq.h"
# else
#include "../../arch/riscv32/include/irq.h"
# endif
#endif

/**
 * Initialize arch-specific IRQ stuff.
 *
 * @param fdt Global FDT pointer.
 */
void setup_irq(void *fdt);

/**
 * Activate IRQ for \p id.
 * Assumes there's one interrupt controller for the whole system, which might be
 * an oversimplification.
 *
 * @param id IRQ id to deactivate.
 * @return OK on success, non-zero otherwise.
 */
stat_t activate_irq(irq_t id);

/**
 * Deactivates IRQ for \p id.
 *
 * @param id IRQ id to deactivate.
 * @return OK on success, non-zero otherwise.
 */
stat_t deactivate_irq(irq_t id);

/** Enable IRQs. */
void enable_irqs();

/** Disable IRQs. */
void disable_irqs();

/** Get IRQ to handle.
 *
 * @return ID of IRQ to handle.
 *
 * @todo what about illegal IRQs due to bug or something?
 */
irq_t get_irq();

#endif /* KMI_ARCH_IRQ_H */
