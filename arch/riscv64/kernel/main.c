/**
 * @file main.c
 * riscv64 main
 */

#include <apos/utils.h>
#include <arch/arch.h>
#include <csr.h>

stat_t setup_arch(void *fdt)
{
	UNUSED(fdt);
	/* allow supervisor code to touch user pages */
	csr_set(CSR_SSTATUS, SSTATUS_SUM);
	/* mark that we want to eventually jump to userspace */
	csr_clear(CSR_SSTATUS, SSTATUS_SPP);

	return OK;
}
