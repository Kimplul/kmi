/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file mem.c
 * Generic memory handling, used both by physical and virtual memory.
 */

#include <kmi/types.h>
#include <kmi/mem.h>
#include <kmi/vmem.h>
#include <libfdt.h>

/** Page order shifts. */
static size_t mm_shifts[10];

/** Page order widths. */
static size_t mm_widths[10];

/** Page order sizes. */
static size_t mm_sizes[10];

/** Base page shift, i.e. how many bits are just offsets within a page. */
static size_t mm_page_shift;

/** Maximum order supported by the current cpu. */
static enum mm_order mm_max_order;

size_t order_shift(enum mm_order order)
{
	return mm_shifts[order];
}

size_t order_size(enum mm_order order)
{
	return mm_sizes[order];
}

enum mm_order max_order()
{
	return mm_max_order;
}

size_t order_width(enum mm_order order)
{
	return mm_widths[order];
}

size_t page_shift()
{
	return mm_page_shift;
}

/**
 * RAM base address. Not sure if it should be provided through a macro
 * like __mm_*.
 */
static pm_t ram_base;

/** RAM size. */
static size_t ram_size;

/** Load address. */
static pm_t load_addr;

enum mm_order nearest_order(size_t size)
{
	for (enum mm_order order = max_order(); order >= MM_MIN; --order)
		if (order_size(order) >= size)
			return order;

	return MM_O0;
}

void init_mem(void *fdt)
{
	size_t top_order = 0;
	size_t base_bits = 0;
	size_t bits[NUM_ORDERS] = { 0 };
	stat_pmem_conf(fdt, &top_order, &base_bits, bits);

	mm_max_order = top_order;
	mm_page_shift = base_bits;

	mm_shifts[0] = mm_page_shift;
	mm_widths[0] = 1 << bits[0];
	mm_sizes[0] = 1 << mm_page_shift;

	for (enum mm_order i = MM_O1; i <= max_order(); ++i) {
		mm_widths[i] = 1 << bits[i];
		mm_shifts[i] = mm_shifts[i - 1] + bits[i - 1];
		mm_sizes[i] = 1UL << mm_shifts[i];
	}
}

void set_ram_base(pm_t base)
{
	ram_base = base;
}

void set_ram_size(size_t size)
{
	ram_size = size;
}

void set_load_addr(pm_t addr)
{
	load_addr = addr;
}

pm_t get_ram_base()
{
	return ram_base;
}

size_t get_ram_size()
{
	return ram_size;
}

uintptr_t get_load_addr()
{
	return load_addr;
}
