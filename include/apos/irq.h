#ifndef APOS_IRQ_H
#define APOS_IRQ_H

void init_irq(void *fdt);
void handle_irq();
void enable_irq();
void disable_irq();

#endif /* APOS_IRQ_H */
