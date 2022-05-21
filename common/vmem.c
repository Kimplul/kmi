#include <apos/mem_regions.h>
#include <apos/assert.h>
#include <apos/debug.h>
#include <apos/bits.h>
#include <apos/vmem.h>
#include <arch/vmem.h>

stat_t init_uvmem(struct tcb *t, vm_t base, vm_t top)
{
	return init_region(&t->sp_r, base, top);
}

vm_t alloc_uvmem(struct tcb *t, size_t size, vmflags_t flags)
{
	/* t exists and is the root tcb of the current process */
	hard_assert(t && !t->parent, ERR_INVAL);

	stat_t status = OK;
	const vm_t v = alloc_region(&t->sp_r, size, &size, flags);
	const vm_t w = map_allocd_region(t->b_r, v, size, flags, &status);
	if (status == INFO_SEFF)
		clone_tcb_maps(t);

	return w;
}

vm_t alloc_fixed_uvmem(struct tcb *t, vm_t start, size_t size, vmflags_t flags)
{
	hard_assert(t && !t->parent, ERR_INVAL);

	stat_t status = OK;
	const vm_t v = alloc_fixed_region(&t->sp_r, start, size, &size, flags);
	const vm_t w = map_allocd_region(t->b_r, v, size, flags, &status);

	if (status == INFO_SEFF)
		clone_tcb_maps(t);

	return w;
}

/* free_shared_uvmem shouldn't be needed, likely to work with free_uvmem */
vm_t alloc_shared_uvmem(struct tcb *t, size_t size, vmflags_t flags)
{
	hard_assert(t && t->parent, ERR_INVAL);

	stat_t status = OK;
	const vm_t v = alloc_region(&t->sp_r, size, &size,
	                            flags | MR_SHARED | MR_OWNED);
	const vm_t w = map_shared_region(t->b_r, v, size, flags, &status);

	if (status == INFO_SEFF)
		clone_tcb_maps(t);

	return w;
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

/* TODO: assume tcb is root tcb? */
stat_t free_uvmem(struct tcb *t, vm_t va)
{
	struct mem_region *m = find_used_region(&t->sp_r, va);
	if (!m)
		return -1;

	pm_t pa = __addr(m->end - m->start);

	vmflags_t flags = m->flags;
	free_region(&t->sp_r, va);

	stat_t status = OK;
	unmap_freed_region(t->b_r, va, pa, flags, &status);
	if (status == INFO_SEFF)
		return clone_tcb_maps(t);

	return status;
}

stat_t alloc_uvmem_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr,
                           vmflags_t flags, enum mm_order order, void *data)
{
	*offset = alloc_page(order, *offset);
	if (!*offset)
		return INFO_TRGN; /* try again */

	stat_t ret = map_vpage(b, *offset, vaddr, flags, order);
	return ret;
}

stat_t alloc_shared_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr,
                            vmflags_t flags, enum mm_order order, void *data)
{
	if (order != MM_O0)
		return INFO_TRGN;

	*offset = alloc_page(MM_O0, *offset);
	map_vpage(b, *offset, vaddr, flags, order);
	return OK;
}

stat_t free_uvmem_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr,
                          vmflags_t flags, enum mm_order order, void *data)
{
	UNUSED(flags);
	UNUSED(offset);

	pm_t paddr = 0;
	enum mm_order v_order = 0;
	stat_vpage(b, vaddr, &paddr, &v_order, 0);
	if (order != v_order)
		return INFO_TRGN;

	stat_t *status = (stat_t *)data;
	*status = unmap_vpage(b, vaddr);
	/* don't free shared pages, unless they're owned */
	if (!__is_set(flags, MR_SHARED) || __is_set(flags, MR_OWNED))
		free_page(order, paddr);

	return OK;
}
