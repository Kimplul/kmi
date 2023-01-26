/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file mem.c
 * Generic memory handling, used both by physical and virtual memory.
 */

#include <apos/types.h>
#include <apos/mem.h>
#include <apos/vmem.h>
#include <libfdt.h>

size_t __mm_shifts[10];
size_t __mm_widths[10];
size_t __mm_sizes[10];
size_t __mm_page_shift;
enum mm_order __mm_max_order;

enum mm_order nearest_order(size_t size)
{
	for (enum mm_order order = max_order(); order >= MM_MIN; --order)
		if (order_size(order) >= size)
			return order;

	return MM_O0;
}

void init_mem(size_t max_order, size_t bits[10], size_t page_shift)
{
	__mm_max_order = max_order;
	__mm_page_shift = page_shift;

	__mm_shifts[0] = page_shift;
	__mm_widths[0] = 1 << bits[0];
	__mm_sizes[0] = 1 << __mm_page_shift;

	for (enum mm_order i = MM_O1; i <= max_order(); ++i) {
		__mm_widths[i] = 1 << bits[i];
		__mm_shifts[i] = __mm_shifts[i - 1] + bits[i - 1];
		__mm_sizes[i] = 1UL << __mm_shifts[i];
	}
}
