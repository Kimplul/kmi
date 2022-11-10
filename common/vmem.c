/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file vmem.c
 * Virtual memory handling, mainly userspace virtual memory.
 */

#include <apos/mem_regions.h>
#include <apos/assert.h>
#include <apos/string.h>
#include <apos/debug.h>
#include <apos/bits.h>
#include <apos/vmem.h>
#include <arch/vmem.h>

stat_t init_uvmem(struct tcb *t, vm_t base, vm_t top)
{
	return init_region(&t->sp_r, base, top);
}

static stat_t __clone_mapped_region(struct tcb *d, struct tcb *s, struct mem_region *m)
{
	size_t size = 0;
	vm_t va = alloc_fixed_region(&d->sp_r, m->start, m->end - m->start,
			&size, m->flags);

	catastrophic_assert(va == m->start);

	if (copy_allocd_region(d->proc.vmem, va, size, m->flags, s))
		return ERR_MISC;

	return OK;
}

/**
 * Convenience function for freeing mapped regions.
 *
 * @param t Thread to work in.
 * @param m Memory region to free.
 * @return \ref INFO_SEFF if other thread in process should be synced, \ref OK
 * otherwise.
 */
static stat_t __free_mapped_region(struct tcb *t, struct mem_region *m)
{
	stat_t status = OK;
	pm_t pa = __addr(m->end - m->start);
	if (unmap_freed_region(t->proc.vmem, m->start, pa, m->flags, &status))
		return ERR_MISC;

	return status;
}

stat_t clear_uvmem(struct tcb *t)
{
	struct mem_region *m = find_first_region(&t->sp_r);
	while (m) {
		if (is_region_owned(m) && !is_region_kept(m))
			__free_mapped_region(t, m);

		m = m->next;
	}

	return OK;
}

stat_t purge_uvmem(struct tcb *t)
{
	struct mem_region *m = find_first_region(&t->sp_r);
	while (m) {
		if (is_region_owned(m))
			__free_mapped_region(t, m);

		m = m->next;
	}

	return OK;
}

stat_t destroy_uvmem(struct tcb *t)
{
	/* force clear all regions */
	purge_uvmem(t);
	/* destroy region tree itself */
	return destroy_region(&t->sp_r);
}

stat_t clone_mem_regions(struct tcb *d, struct tcb *s)
{
	/** @todo implement some way to only iterate used regions, this loops
	 * through all regions which is likely a slight bit slower. */
	struct mem_region *m = find_first_region(&s->sp_r);
	while (m) {
		if (is_region_used(m))
			__clone_mapped_region(d, s, m);

		m = m->next;
	}

	return OK;
}

vm_t alloc_uvmem(struct tcb *t, size_t size, vmflags_t flags)
{
	/* t exists and is the process tcb of the current process */
	hard_assert(t && is_proc(t), ERR_INVAL);

	stat_t status = OK;
	const vm_t v = alloc_region(&t->sp_r, size, &size, flags);
	const vm_t w = map_allocd_region(t->proc.vmem, v, size, flags, &status);
	/** \todo this could be changed so that each thread allocated the memory
	 * region for itself to start with, and only when someone tries to
	 * access it from some other thread, is it actually cloned. Would likely
	 * need some major reworkings, so this is good enough for now. */
	if (is_rpc(t) && status == INFO_SEFF)
		clone_rpc_maps(t);

	return w;
}

vm_t alloc_fixed_uvmem(struct tcb *t, vm_t start, size_t size, vmflags_t flags)
{
	hard_assert(t && is_proc(t), ERR_INVAL);

	stat_t status = OK;
	const vm_t v = alloc_fixed_region(&t->sp_r, start, size, &size, flags);
	const vm_t w = map_allocd_region(t->proc.vmem, v, size, flags, &status);

	if (is_rpc(t) && status == INFO_SEFF)
		clone_rpc_maps(t);

	return w;
}

/* free_shared_uvmem shouldn't be needed, likely to work with free_uvmem */
vm_t alloc_shared_uvmem(struct tcb *t, size_t size, vmflags_t flags)
{
	hard_assert(t && is_proc(t), ERR_INVAL);

	stat_t status = OK;
	const vm_t v = alloc_region(&t->sp_r, size, &size,
	                            flags | MR_SHARED | MR_OWNED);
	const vm_t w = map_shared_region(t->proc.vmem, v, size, flags, &status);

	if (is_rpc(t) && status == INFO_SEFF)
		clone_rpc_maps(t);

	return w;
}

vm_t ref_shared_uvmem(struct tcb *t1, struct tcb *t2, vm_t va, vmflags_t flags)
{
	struct mem_region *m = find_used_region(&t1->sp_r, va);
	if (!m)
		return 0;

	if (!is_set(m->flags, MR_OWNED))
		return 0;

	/* loop over all pages in this region */
	size_t size = m->end - m->start;
	size_t pages = __pages(size);
	vm_t v = alloc_region(&t2->sp_r, size, &size, flags | MR_SHARED);
	/* a for each page could be cool? */
	vm_t runner = v;
	for (; pages; --pages) {
		pm_t paddr;
		stat_vpage(t1->proc.vmem, v, &paddr, 0, 0);
		map_vpage(t2->proc.vmem, paddr, runner, flags, MM_O0);
		runner += order_size(MM_O0);
	}

	return v;
}

stat_t free_uvmem(struct tcb *r, vm_t va)
{
	/** \todo assume tcb is root tcb? */
	struct mem_region *m = find_used_region(&r->sp_r, va);
	if (!m)
		return ERR_NF;

	free_region(&r->sp_r, va);

	stat_t status = __free_mapped_region(r, m);
	if (is_rpc(r) && status == INFO_SEFF)
		return clone_rpc_maps(r);

	return status;
}

stat_t alloc_uvmem_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                           vmflags_t flags, enum mm_order order, void *data)
{
	*offset = alloc_page(order);
	if (!*offset)
		return INFO_TRGN; /* try again */

	stat_t *status = (stat_t *)data, ret;
	ret = map_vpage(b, *offset, vaddr, flags, order);
	if (status)
		*status = ret;

	return (ret == INFO_SEFF) ? OK : ret;
}

stat_t alloc_shared_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                            vmflags_t flags, enum mm_order order, void *data)
{
	if (order != MM_O0)
		return INFO_TRGN;

	*offset = alloc_page(MM_O0);

	stat_t *status = (stat_t *)data, ret;
	ret = map_vpage(b, *offset, vaddr, flags, order);
	if (status)
		*status = ret;
	return (ret == INFO_SEFF) ? OK : ret;
}

stat_t copy_allocd_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
		vmflags_t flags, enum mm_order order, void *data)
{
	struct vmem *s = (struct vmem *)data;

	pm_t paddr = 0;
	enum mm_order v_order = 0;
	stat_vpage(s, vaddr, &paddr, &v_order, 0);
	/** @todo what if we could combine multiple pages into one in the new
	 * process? */
	if (order > v_order)
		return INFO_TRGN;

	pm_t new_page = alloc_page(order);
	if (!new_page)
		return INFO_TRGN;

	map_vpage(b, new_page, vaddr, flags, order);
	memcpy((void *)new_page, (void *)(paddr + *offset), order_size(order));

	if (v_order > order)
		*offset += order_size(order);
	else
		*offset = 0;

	return OK;
}

stat_t free_uvmem_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                          vmflags_t flags, enum mm_order order, void *data)
{
	UNUSED(flags);
	UNUSED(offset);

	pm_t paddr = 0;
	enum mm_order v_order = 0;
	stat_vpage(b, vaddr, &paddr, &v_order, 0);
	if (order != v_order)
		return INFO_TRGN;

	stat_t *status = (stat_t *)data, ret;
	ret = unmap_vpage(b, vaddr);
	if (status)
		*status = ret;

	/* don't free shared pages, unless they're owned */
	if (!is_set(flags, MR_SHARED) || is_set(flags, MR_OWNED))
		free_page(order, paddr);

	return (ret == INFO_SEFF) ? OK : ret;
}
