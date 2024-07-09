/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file dmem.c
 * Handle device memory, i.e. anything outside the physical RAM.
 *
 * \todo Handle NUMA.
 */

#include <kmi/assert.h>
#include <kmi/dmem.h>

/** Region before RAM. */
static struct mem_region_root pre_ram = { 0 };

/** Region after RAM. */
static struct mem_region_root post_ram = { 0 };

stat_t init_devmem(pm_t ram_base, pm_t ram_top)
{
	size_t pre_pages = __pages(ram_base);
	size_t ram_pages = __pages(ram_top - ram_base);
	/* -1 being the effective highest address possible */
	size_t post_pages = __pages(-1) - ram_pages - pre_pages;

	init_region(&pre_ram, 0, pre_pages, 0);
	init_region(&post_ram, ram_top, post_pages, 0);
	return OK;
}

/**
 * Select between the area before RAM or after RAM, depending on \p addr.
 *
 * @param addr Physical address of device.
 * @return Corresponding device memory region or NULL if \p addr is within RAM.
 */
static struct mem_region_root *__select_region(pm_t addr)
{
	if (addr < get_ram_base())
		return &pre_ram;

	if (addr > (get_ram_base() + get_ram_size()))
		return &post_ram;

	return NULL;
}

vm_t alloc_devmem(struct tcb *t, pm_t start, size_t bytes, vmflags_t flags)
{
	assert(t && is_proc(t));

	struct mem_region_root *region = __select_region(start);
	if (!region)
		return NULL;

	vm_t v = alloc_region(region, bytes, &bytes, flags);
	if (!v)
		return NULL;

	if (map_fixed_region(t->proc.vmem, v, start, bytes, flags)) {
		unmap_region(t->proc.vmem, v, bytes);
		free_region(region, v);
		return NULL;
	}

	return v;
}

stat_t free_devmem(struct tcb *t, vm_t start)
{
	assert(t && is_proc(t));
	pm_t addr = 0;
	stat_vpage(t->proc.vmem, start, &addr, NULL, NULL);

	struct mem_region_root *region = __select_region((pm_t)__pa(addr));
	if (!region)
		return ERR_INVAL;


	struct mem_region *m = find_used_region(region, start);
	if (!m)
		return ERR_NF;

	vm_t base = __addr(m->start);
	vm_t end = __addr(m->end);
	size_t size = end - base;
	unmap_fixed_region(t->proc.vmem, base, size);

	free_region(region, base);
	return OK;
}
