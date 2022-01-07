#include <apos/mem_regions.h>
#include <apos/vmem.h>


stat_t init_uvmem(struct tcb *t, vm_t base, vm_t top)
{
	return sp_mem_init(&t->sp_r, base, top);
}

vm_t alloc_uvmem(struct tcb *t, size_t size, uint8_t flags)
{
	vm_t v = alloc_region(&t->sp_r, size, &size);
	return map_allocd_region(t->b_r, v, size, flags);
}

vm_t alloc_fixed_uvmem(struct tcb *t, vm_t start, size_t size, uint8_t flags)
{
	vm_t v = alloc_fixed_region(&t->sp_r, start, size, &size);
	return map_allocd_region(t->b_r, v, size, flags);
}

stat_t free_uvmem(struct tcb *t, vm_t va)
{
	struct sp_mem *m = sp_used_find(&t->sp_r, va);
	if(!m)
		return -1;

	pm_t pa = __addr(m->end - m->start);

	free_region(&t->sp_r, va);
	unmap_freed_region(t->b_r, va, pa);
	return 0;
}

stat_t alloc_uvmem_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr, uint8_t flags, enum mm_order order)
{
	*offset = alloc_page(order, *offset);
	if(!*offset)
		return 1; /* try again */

	map_vmem(b, *offset, vaddr, flags, order);
	return 0;
}

stat_t free_uvmem_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr, uint8_t flags, enum mm_order order)
{
	UNUSED(flags); UNUSED(offset);

	pm_t paddr = 0;
	enum mm_order v_order = 0;
	stat_vmem(b, vaddr, &paddr, &v_order, 0);
	if(order != v_order)
		return -1;

	unmap_vmem(b, vaddr);
	free_page(order, paddr);
	return 0;
}
