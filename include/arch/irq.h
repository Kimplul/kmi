#ifndef APOS_IRQ_H
#define APOS_IRQ_H

/**
 * @file irq.h
 * Arch-specific interrupt handling, generally implemented in
 * arch/whatever/kernel/irq.c
 */

#if defined(riscv64)
#include "../../arch/riscv64/include/irq.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/irq.h"
#endif

void init_irq(void *fdt);
void handle_irq();
void enable_irq();
void disable_irq();

#endif /* APOS_IRQ_H */
