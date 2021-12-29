#include <apos/pmem.h>

int arch_pmem_conf(void *fdt, size_t *max_order, size_t *base_bits, size_t bits[ORDERS_NUM])
{
	UNUSED(fdt);

	/* Sv39 for now */
	*max_order = 2;
	*base_bits = 12;
	/* we can assume bits[] is zeroed out beforehand */
	for(size_t i = 0; i <= *max_order; ++i)
		bits[i] = 9;

	return 0;
}
