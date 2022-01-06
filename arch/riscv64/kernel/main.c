#include <apos/utils.h>
#include <csr.h>

void arch_setup(void *fdt)
{
	UNUSED(fdt);
	/* allow supervisor code to touch user pages */
	csr_set(CSR_SSTATUS, SSTATUS_SUM);
	/* mark that we want to eventually jump to userspace */
	csr_clear(CSR_SSTATUS, SSTATUS_SPP);
}
