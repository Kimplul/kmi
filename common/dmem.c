/**
 * @file dmem.c
 * Handle device memory, i.e. anything outside the physical RAM.
 *
 * \todo Handle NUMA.
 */

#include <apos/assert.h>
#include <apos/dmem.h>

static struct mem_region_root pre_ram = { 0 };
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

static stat_t dev_alloc_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                                vmflags_t flags, enum mm_order order,
                                void *data)
{
	stat_t *status = (stat_t *)data;
	/* TODO: remember to do something with this status info */
	*status = map_vpage(b, *offset, vaddr, flags, order);
	*offset += order_size(order);
	return OK;
}

static stat_t dev_free_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                               vmflags_t flags, enum mm_order order, void *data)
{
	UNUSED(offset);
	UNUSED(flags);
	pm_t paddr = 0;
	enum mm_order v_order = 0;
	stat_vpage(b, vaddr, &paddr, &v_order, 0);
	if (order != v_order)
		return INFO_TRGN;

	stat_t *status = (stat_t *)data;
	*status = unmap_vpage(b, vaddr);
	return OK;
}

vm_t alloc_devmem(struct tcb *t, pm_t dev_start, size_t bytes, vmflags_t flags)
{
	hard_assert(t && is_proc(t), ERR_INVAL);

	vm_t region = 0;
	if (dev_start < __pre_top)
		region = alloc_region(&pre_ram, bytes, 0, flags);

	if (dev_start > __post_base)
		region = alloc_region(&post_ram, bytes, 0, flags);

	if (!region)
		return 0;

	stat_t status = OK;
	const vm_t w = map_fill_region(t->proc.vmem, &dev_alloc_wrapper,
	                               dev_start, region,
	                               bytes, flags, &status);
	if (is_rpc(t) && status == INFO_SEFF)
		clone_rpc_maps(t);

	return w;
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
		m = find_used_region(&pre_ram, dev_paddr);

	if (dev_paddr > __post_base)
		m = find_used_region(&post_ram, dev_paddr);

	if (!m)
		return ERR_NF;

	size_t region_size = __addr(m->end - m->start);
	stat_t status = OK;
	map_fill_region(t->proc.vmem, &dev_free_wrapper, dev_paddr, dev_start,
	                region_size, 0, &status);
	if (is_rpc(t) && status == INFO_SEFF)
		clone_rpc_maps(t);

	if (dev_paddr < __pre_top)
		free_region(&pre_ram, dev_paddr);

	if (dev_paddr > __post_base)
		free_region(&post_ram, dev_paddr);

	return OK;
}
