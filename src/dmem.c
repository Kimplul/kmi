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

pm_t __pre_base = 0;
pm_t __pre_top = 0;
pm_t __post_base = 0;
pm_t __post_top = 0;

stat_t init_devmem(pm_t ram_base, pm_t ram_top)
{
	pm_t mem_top = (pm_t)-1;

	__pre_base = 0;
	__pre_top = ram_base - 1;

	__post_base = ram_top;
	__post_top = mem_top;

	size_t pre_pages = __pages(__pre_top);
	size_t post_pages = __pages(__post_top) - __pages(__post_base);

	init_region(&pre_ram, __pre_base, pre_pages);
	init_region(&post_ram, __post_base, post_pages);

	return OK;
}

vm_t alloc_devmem(struct tcb *t, pm_t dev_start, size_t bytes, vmflags_t flags)
{
	hard_assert(t && is_proc(t), ERR_INVAL);

	struct mem_region_root *region = NULL;
	if (dev_start < __pre_top)
		region = &pre_ram;

	else if (dev_start > __post_base)
		region = &post_ram;
	else
		return NULL;

	vm_t v = alloc_region(region, bytes, &bytes, flags);
	if (!v)
		return NULL;

	if (map_fixed_region(t->proc.vmem, v, dev_start, bytes, flags)) {
		unmap_region(t->proc.vmem, v, bytes);
		free_region(region, v);
		return NULL;
	}

	return v;
}

stat_t free_devmem(struct tcb *t, vm_t dev_start)
{
	hard_assert(t && is_proc(t), ERR_INVAL);

	pm_t dev_paddr = 0;
	stat_vpage(t->proc.vmem, dev_start, &dev_paddr, 0, 0);

	if (dev_paddr >= __pre_top && dev_paddr <= __post_base)
		return ERR_ADDR;

	struct mem_region *m = 0;
	if (dev_paddr < __pre_top)
		m = find_used_region(&pre_ram, dev_start);

	else if (dev_paddr > __post_base)
		m = find_used_region(&post_ram, dev_start);

	if (!m)
		return ERR_NF;

	vm_t start = __addr(m->start);
	vm_t end = __addr(m->end);
	size_t size = end - start;
	unmap_fixed_region(t->proc.vmem, start, size);

	if (dev_paddr < __pre_top)
		free_region(&pre_ram, dev_start);

	else if (dev_paddr > __post_base)
		free_region(&post_ram, dev_start);

	return OK;
}
