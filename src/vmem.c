/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file vmem.c
 * Virtual memory handling, mainly userspace virtual memory.
 */

#include <kmi/mem_regions.h>
#include <kmi/assert.h>
#include <kmi/string.h>
#include <kmi/debug.h>
#include <kmi/bits.h>
#include <kmi/vmem.h>
#include <arch/vmem.h>

stat_t init_uvmem(struct tcb *t, vm_t base, vm_t top)
{
	return init_region(&t->sp_r, base, top);
}

/**
 * Clone process memory region.
 *
 * @param d Destination tcb.
 * @param s Source tcb.
 * @param m Memory region to clone.
 * @return \ref ERR_MISC if clone failed, otherwise \ref OK.
 *
 * @todo check shared memory regions.
 */
static stat_t __clone_mapped_region(struct tcb *d, struct tcb *s,
                                    struct mem_region *m)
{
	vm_t start = m->start * order_size(BASE_PAGE);
	vm_t end = m->end * order_size(BASE_PAGE);

	size_t size = end - start, actual_size = 0;
	vm_t va = alloc_fixed_region(&d->sp_r, start, size,
	                             &actual_size, m->flags);

	catastrophic_assert(va == start);

	if (!copy_allocd_region(d->proc.vmem, va, size, m->flags, s->proc.vmem))
		return ERR_MISC;

	return OK;
}

/**
 * Unmap and free private memory region.
 *
 * @param t Current thread.
 * @param m Memory region to free.
 * @return \see unmap_freed_region().
 */
static stat_t __free_mapped_private_region(struct tcb *t, struct mem_region *m)
{
	stat_t status = OK;
	pm_t start = __addr(m->start);
	pm_t end = __addr(m->end);
	if (!unmap_freed_region(t->proc.vmem, start, end - start, m->flags,
	                        &status))
		return ERR_MISC;

	return status;
}

/**
 * Check whether process associated with shared memory is still using it.
 *
 * @param pid Process to check.
 * @param start Start of memory region
 * @return \ref true if it is still in use, \ref false otherwise.
 */
static bool __proc_has_region(id_t pid, vm_t start)
{
	/** @todo this has a slight potential to have a race condition, where
	 * both threads want to free the same shared region at the same time. */
	struct tcb *p = get_tcb(pid);
	if (!p)
		return false;

	struct mem_region *m = find_used_region(&p->sp_r, start);
	if (!m)
		return false;

	return true;
}

/**
 * Unmap shared region and free associated physical pages if they're not being
 * used by the other process.
 *
 * @param t Current thread.
 * @param m Memory region to free.
 * @return \see unmap_vpage().
 */
static stat_t __free_mapped_shared_region(struct tcb *t, struct mem_region *m)
{
	vm_t start = __addr(m->start);
	bool in_use = __proc_has_region(m->pid, m->alt_va);

	size_t osize = order_size(BASE_PAGE);
	size_t pages = m->start - m->end;

	stat_t status = OK;
	for (size_t i = 0; i < pages; ++i) {
		vm_t va = start + i * osize;

		pm_t pa = 0;
		stat_vpage(t->proc.vmem, va, &pa, 0, 0);
		status = unmap_vpage(t->proc.vmem, va);

		if (!in_use)
			free_page(pa, BASE_PAGE);
	}

	return status;
}

/**
 * Convenience function for freeing mapped regions.
 *
 * @param t Thread to work in.
 * @param m Memory region to free.
 * @return \ref OK
 */
static stat_t __free_mapped_region(struct tcb *t, struct mem_region *m)
{
	if (m->pid != 0)
		return __free_mapped_shared_region(t, m);

	return __free_mapped_private_region(t, m);
}

stat_t clear_uvmem(struct tcb *t)
{
	struct mem_region *m = find_first_region(&t->sp_r);
	while (m) {
		if (!is_region_kept(m))
			__free_mapped_region(t, m);

		m = m->next;
	}

	return OK;
}

stat_t purge_uvmem(struct tcb *t)
{
	struct mem_region *m = find_first_region(&t->sp_r);
	while (m) {
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
	return w;
}

vm_t alloc_uvpage(struct tcb *t, size_t size, vmflags_t flags, size_t *asize,
                  pm_t *paddr)
{
	hard_assert(t && is_proc(t), ERR_INVAL);

	enum mm_order order = nearest_order(size);
	size_t actual_size = order_size(order);
	stat_t status = OK;

	const vm_t v = alloc_region(&t->sp_r, size, &size, flags);
	const vm_t w = __addr(__page(v));

	pm_t addr = alloc_page(order);
	/** @todo should free region */
	if (!addr)
		return NULL;

	status = map_vpage(t->proc.vmem, addr, w, flags, order);
	if (status)
		return NULL;

	if (asize)
		*asize = actual_size;

	if (paddr)
		*paddr = addr;

	return w;
}

vm_t alloc_fixed_uvmem(struct tcb *t, vm_t start, size_t size, vmflags_t flags)
{
	hard_assert(t && is_proc(t), ERR_INVAL);

	stat_t status = OK;
	const vm_t v = alloc_fixed_region(&t->sp_r, start, size, &size, flags);
	const vm_t w = map_allocd_region(t->proc.vmem, v, size, flags, &status);
	return w;
}

static vm_t map_fixed_region(struct vmem *b, vm_t v, pm_t p, size_t size,
                             vmflags_t flags, stat_t *status)
{

	vm_t w = v;
	stat_t stat = OK;
	size_t pages = size / BASE_PAGE_SIZE;
	for (size_t i = 0; i < pages; ++i) {
		stat = map_vpage(b, p, v, flags, BASE_PAGE);
		v += BASE_PAGE_SIZE;
		p += BASE_PAGE_SIZE;
	}

	if (status)
		*status = stat;

	return w;
}

vm_t map_fixed_mem(struct tcb *t, pm_t start, size_t size, vmflags_t flags)
{
	stat_t status = OK;
	const vm_t v = alloc_region(&t->sp_r, size, &size, flags);
	const vm_t w = map_fixed_region(t->proc.vmem, v, start, size, flags,
	                                &status);
	return w + (start % BASE_PAGE_SIZE);
}

/* free_shared_uvmem shouldn't be needed, likely to work with free_uvmem */
stat_t alloc_shared_uvmem(struct tcb *s, struct tcb *c,
                          size_t size, vmflags_t sflags, vmflags_t cflags,
                          vm_t *sstart, vm_t *cstart)
{
	hard_assert(sstart, ERR_INVAL);
	hard_assert(cstart, ERR_INVAL);
	hard_assert(s && is_proc(s), ERR_INVAL);
	hard_assert(c && is_proc(c), ERR_INVAL);

	size_t ssize, csize;
	vm_t sv = alloc_shared_region(&s->sp_r, size, &ssize, sflags, c->rid);
	vm_t cv = alloc_shared_region(&c->sp_r, size, &csize, cflags, s->rid);

	/* not exactly optimal but good enough for now, I can start worrying
	 * about hyperoptimizations whenever. */
	set_alt_region_addr(&s->sp_r, sv, cv);
	set_alt_region_addr(&c->sp_r, cv, sv);

	if (csize != ssize) {
		/** @todo cleanup, better errors? */
		return ERR_INVAL;
	}

	stat_t cstatus = OK, sstatus = OK;
	size_t osize = order_size(BASE_PAGE);
	size_t pages = ssize / osize;
	for (size_t i = 0; i < pages; ++i) {
		pm_t p = alloc_page(BASE_PAGE);
		sstatus = map_vpage(s->proc.vmem, p, sv + i * osize, sflags,
		                    BASE_PAGE);
		cstatus = map_vpage(c->proc.vmem, p, cv + i * osize, cflags,
		                    BASE_PAGE);
	}

	*sstart = sv;
	*cstart = cv;

	if (sstatus)
		return sstatus;

	if (cstatus)
		return cstatus;

	return OK;
}

stat_t free_uvmem(struct tcb *r, vm_t va)
{
	/** \todo assume tcb is root tcb? */
	struct mem_region *m = find_used_region(&r->sp_r, va);
	if (!m)
		return ERR_NF;

	stat_t status = __free_mapped_region(r, m);
	if (status)
		return ERR_MISC;

	return free_known_region(&r->sp_r, m);
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

	return ret;
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

	return ret;
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

	/* set write flags temporarily */
	vmflags_t wrflags = flags | VM_W;
	map_vpage(b, new_page, vaddr, wrflags, order);
	memcpy((void *)new_page, (void *)(paddr + *offset), order_size(order));

	/* set actual flags */
	map_vpage(b, new_page, vaddr, flags, order);

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

	/** @todo we might need to cause an ipi to flush the tlb for other
	 * cores */

	stat_t *status = (stat_t *)data, ret;
	ret = unmap_vpage(b, vaddr);
	if (status)
		*status = ret;

	free_page(order, paddr);

	return ret;
}
