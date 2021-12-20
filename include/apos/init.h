#ifndef APOS_INIT_H
#define APOS_INIT_H

#include <apos/pmem.h>
#include <apos/vmem.h>

struct init_data_t {
	pm_t init_base;
	pm_t init_top;

	pm_t initrd_base;
	pm_t initrd_top;

	pm_t pmap_base;
	pm_t pmap_top;

	pm_t fdt_base;
	pm_t fdt_top;

	pm_t stack_base;
	pm_t stack_top;

	struct vm_branch_t *kernel_vm_base;

	size_t max_order;
	size_t bits[10];
	size_t page_shift;
};

#endif /* APOS_INIT_H */
