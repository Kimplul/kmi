#include <apos/mem_regions.h>
#include <apos/bits.h>
#include <apos/vmem.h>
#include <arch/vmem.h>

stat_t init_uvmem(struct tcb *t, vm_t base, vm_t top)
{
	return init_region(&t->sp_r, base, top);
}

vm_t alloc_uvmem(struct tcb *t, size_t size, vmflags_t flags)
{
	vm_t v = alloc_region(&t->sp_r, size, &size, flags);
	return map_allocd_region(t->b_r, v, size, flags);
}

vm_t alloc_fixed_uvmem(struct tcb *t, vm_t start, size_t size, vmflags_t flags)
{
	vm_t v = alloc_fixed_region(&t->sp_r, start, size, &size, flags);
	return map_allocd_region(t->b_r, v, size, flags);
}

/* free_shared_uvmem shouldn't be needed, likely to work with free_uvmem */
vm_t alloc_shared_uvmem(struct tcb *t, size_t size, vmflags_t flags)
{
	/* TODO: proper error handling */
	vm_t v = alloc_region(&t->sp_r, size, &size,
	                      flags | MR_SHARED | MR_OWNED);
	return map_shared_region(t->b_r, v, size, flags);
}

vm_t ref_shared_uvmem(struct tcb *t1, struct tcb *t2, vm_t va, vmflags_t flags)
{
	struct mem_region *m = find_used_region(&t1->sp_r, va);
	if (!m)
		return 0;

	if (!__is_set(m->flags, MR_OWNED))
		return 0;

	/* loop over all pages in this region */
	size_t size = m->end - m->start;
	size_t pages = __pages(size);
	vm_t v = alloc_region(&t2->sp_r, size, &size, flags | MR_SHARED);
	/* a for each page could be cool? */
	vm_t runner = v;
	for (; pages; --pages) {
		pm_t paddr;
		stat_vpage(t1->b_r, v, &paddr, 0, 0);
		map_vpage(t2->b_r, paddr, runner, flags, MM_O0);
		runner += __o_size(MM_O0);
	}

	return v;
}

stat_t free_uvmem(struct tcb *t, vm_t va)
{
	struct mem_region *m = find_used_region(&t->sp_r, va);
	if (!m)
		return -1;

	pm_t pa = __addr(m->end - m->start);

	vmflags_t flags = m->flags;
	free_region(&t->sp_r, va);
	unmap_freed_region(t->b_r, va, pa, flags);
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

stat_t alloc_shared_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr,
                            vmflags_t flags, enum mm_order order)
{
	if (order != MM_O0)
		return REGION_TRY_AGAIN;

	*offset = alloc_page(MM_O0, *offset);
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
	/* don't free shared pages, unless they're owned */
	if (!__is_set(flags, MR_SHARED) || __is_set(flags, MR_OWNED))
		free_page(order, paddr);
	return OK;
}
