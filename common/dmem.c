#include <apos/dmem.h>

static struct sp_reg_root pre_ram = {0};
static struct sp_reg_root post_ram = {0};
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

	sp_mem_init(&pre_ram, __pre_base, pre_pages);
	sp_mem_init(&post_ram, __post_base, post_pages);

	return OK;
}

stat_t dev_alloc_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr, vmflags_t flags, enum mm_order order)
{
	map_vpage(b, *offset, vaddr, flags, order);
	*offset += __o_size(order);
	return 0;
}

stat_t dev_free_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr, vmflags_t flags, enum mm_order order)
{
	UNUSED(offset); UNUSED(flags);
	pm_t paddr = 0;
	enum mm_order v_order = 0;
	stat_vpage(b, vaddr, &paddr, &v_order, 0);
	if(order != v_order)
		return -1;

	unmap_vpage(b, vaddr);
	return 0;
}

vm_t alloc_devmem(struct tcb *t, pm_t dev_start, size_t bytes, vmflags_t flags)
{
	vm_t region = 0;
	if(dev_start < __pre_top)
		region = alloc_region(&pre_ram, bytes, 0);

	if(dev_start > __post_base)
		region = alloc_region(&post_ram, bytes, 0);

	if(!region)
		return 0;

	return map_fill_region(t->b_r, &dev_alloc_wrapper, dev_start, region, bytes, flags);
}

stat_t free_devmem(struct tcb *t, vm_t dev_start)
{
	pm_t dev_paddr = 0;
	stat_vpage(t->b_r, dev_start, &dev_paddr, 0, 0);

	if(dev_paddr >= __pre_top && dev_paddr <= __post_base)
		return ERR_ADDR;

	struct sp_mem *m = 0;
	if(dev_paddr < __pre_top)
		m = sp_used_find(&pre_ram, dev_paddr);
	
	if(dev_paddr > __post_base)
		m = sp_used_find(&post_ram, dev_paddr);

	if(!m)
		return ERR_NF;

	size_t region_size = __addr(m->end - m->start);
	map_fill_region(t->b_r, &dev_free_wrapper, dev_paddr, dev_start, region_size, 0);

	if(dev_paddr < __pre_top)
		free_region(&pre_ram, dev_paddr);

	if(dev_paddr > __post_base)
		free_region(&post_ram, dev_paddr);

	return OK;
}
