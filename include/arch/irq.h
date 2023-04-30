/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_IRQ_H
#define KMI_IRQ_H

/**
 * @file irq.h
 * Arch-specific interrupt handling, generally implemented in
 * arch/whatever/kernel/irq.c
 *
 * \todo These should probably also be stat_t...
 */

#if defined(riscv64)
#include "../../arch/riscv64/include/irq.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/irq.h"
#endif

/**
 * Initialize IRQ subsystem.
 *
 * @param fdt Global FDT pointer.
 */
void init_irq(void *fdt);

/** Enable IRQs. */
void enable_irq();

/** Disable IRQs. */
void disable_irq();

#endif /* KMI_IRQ_H */
