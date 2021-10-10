#ifndef APOS_INIT_H
#define APOS_INIT_H

#include <apos/pmem.h>

struct init_data_t {
	paddr_t init_base;
	paddr_t init_top;

	paddr_t initrd_base;
	paddr_t initrd_top;

	paddr_t fdt_base;
	paddr_t fdt_top;

	paddr_t stack_base;
	paddr_t stack_top;

	struct vm_branch_t *kernel_vm_base;

	/*
	   TODO: is this necessary?
	   paddr_t pmap_base;
	   paddr_t pmap_top;
	*/
};

#endif /* APOS_INIT_H */
