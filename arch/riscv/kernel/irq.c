#include <apos/irq.h>
#include <csr.h>

void init_irq(void *fdt)
{
	UNUSED(fdt);
}

void handle_irq()
{
}

/* very simple for now */
void enable_irq()
{
	csr_set(CSR_SSTATUS, CSR_SIE);
}

void disable_irq()
{
	csr_clear(CSR_SSTATUS, CSR_SIE);
}
