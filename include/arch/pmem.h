#ifndef APOS_ARCH_PMEM_H
#define APOS_ARCH_PMEM_H

#include <apos/mem.h> /* NUM_ORDERS */
#include <apos/types.h>
#include <pmem.h>

stat_t stat_pmem_conf(void *fdt, size_t *max_order,
		size_t *base_bits, size_t bits[NUM_ORDERS]);

#endif /* APOS_ARCH_PMEM_H */
