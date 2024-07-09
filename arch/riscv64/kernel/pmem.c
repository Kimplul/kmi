/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file pmem.c
 * riscv64 implementation of arch-specific physical memory handling
 */

#include <kmi/syscalls.h>
#include <arch/pmem.h>

stat_t stat_pmem_conf(void *fdt, size_t *max_order, size_t *base_bits,
                      size_t bits[NUM_ORDERS])
{
	UNUSED(fdt);
	*base_bits = 12;

	size_t nbits = 9;

#if __riscv_xlen == 64
	*max_order = 2;
	nbits = 9;
#else
	*max_order = 1;
	nbits = 10;
#endif

	/* we can assume bits[] is zeroed out beforehand */
	for (size_t i = 0; i <= *max_order; ++i)
		bits[i] = nbits;

	return OK;
}
