#include <apos/mem_regions.h>
#include <apos/vmem.h>
#include <arch/vmem.h>

stat_t init_uvmem(struct tcb *t, vm_t base, vm_t top)
{
	return init_region(&t->sp_r, base, top);
}

vm_t alloc_uvmem(struct tcb *t, size_t size, vmflags_t flags)
{
	vm_t v = alloc_region(&t->sp_r, size, &size);
	return map_allocd_region(t->b_r, v, size, flags);
}

vm_t alloc_fixed_uvmem(struct tcb *t, vm_t start, size_t size, vmflags_t flags)
{
	vm_t v = alloc_fixed_region(&t->sp_r, start, size, &size);
	return map_allocd_region(t->b_r, v, size, flags);
}

stat_t free_uvmem(struct tcb *t, vm_t va)
{
	struct mem_region *m = find_used_region(&t->sp_r, va);
	if (!m)
		return -1;

	pm_t pa = __addr(m->end - m->start);

	free_region(&t->sp_r, va);
	unmap_freed_region(t->b_r, va, pa);
	return 0;
}

stat_t alloc_uvmem_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr,
                           vmflags_t flags, enum mm_order order)
{
	*offset = alloc_page(order, *offset);
	if (!*offset)
		return REGION_TRY_AGAIN; /* try again */

	map_vpage(b, *offset, vaddr, flags, order);
	return OK;
}

stat_t free_uvmem_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr,
                          vmflags_t flags, enum mm_order order)
{
	UNUSED(flags);
	UNUSED(offset);

	pm_t paddr = 0;
	enum mm_order v_order = 0;
	stat_vpage(b, vaddr, &paddr, &v_order, 0);
	if (order != v_order)
		return REGION_TRY_AGAIN;

	unmap_vpage(b, vaddr);
	free_page(order, paddr);
	return OK;
}
