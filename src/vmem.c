/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file vmem.c
 * Virtual memory handling, mainly userspace virtual memory.
 */

#include <kmi/regions.h>
#include <kmi/assert.h>
#include <kmi/string.h>
#include <kmi/debug.h>
#include <kmi/bits.h>
#include <kmi/vmem.h>
#include <arch/vmem.h>

stat_t init_uvmem(struct tcb *t, vm_t base, vm_t top)
{
	t->uvmem.owner = t->tid;
	t->uvmem.vmem = t->proc.vmem;
	return init_region(&t->uvmem.region, base, top);
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
static stat_t __copy_mapped_region(struct tcb *d, struct tcb *s,
                                   struct mem_region *m)
{
	vm_t start = m->start * order_size(BASE_PAGE);
	vm_t end = m->end * order_size(BASE_PAGE);

	size_t size = end - start;
	vm_t v = alloc_fixed_region(&d->uvmem.region, start, size, &size,
	                            m->flags);
	catastrophic_assert(v == start);
	stat_t res = copy_region(d->proc.vmem, s->proc.vmem, v, v, size);
	if (res == OK)
		return OK;

	/* cleanup on error */
	free_region(&d->uvmem.region, v);
	unmap_region(d->proc.vmem, v, size);
	return res;
}

/**
 * Helper for implementing shared memory.
 *
 * @param d 'Destination' of new mapping.
 * @param s Owner of shared region.
 * @param m Shared memory region to clone.
 * @param flags Flags of new mapping.
 * @return Address of new mapping in \p d.
 */
static vm_t __clone_shared_region(struct tcb *d, struct tcb *s,
                                  struct mem_region *m, vmflags_t flags)
{
	vm_t start = m->start * BASE_PAGE_SIZE;
	vm_t end = m->end * BASE_PAGE_SIZE;

	reference_proc(s);

	size_t size = end - start;
	vm_t v = alloc_shared_region(&d->uvmem.region, size, &size, m->flags,
	                             s->rid);
	stat_t res = clone_region(d->proc.vmem, s->proc.vmem, start, v, size,
	                          flags);
	if (res == OK)
		return v;

	/* cleanup on error */
	unreference_proc(s);
	free_region(&d->uvmem.region, v);
	unmap_fixed_region(d->proc.vmem, v, size);
	return NULL;
}

/**
 * Unmap and free private memory region.
 *
 * @param t Current thread.
 * @param m Memory region to free.
 */
static void __free_mapped_private_region(struct tcb *t, struct mem_region *m)
{
	pm_t start = __addr(m->start);
	pm_t end = __addr(m->end);
	size_t size = end - start;
	unmap_region(t->proc.vmem, start, size);
}

/**
 * Unmap shared region and free associated physical pages if they're not being
 * used by the other process.
 *
 * @param t Current thread.
 * @param m Memory region to free.
 */
static void __free_mapped_shared_region(struct tcb *t, struct mem_region *m)
{
	vm_t start = __addr(m->start);
	vm_t end = __addr(m->end);
	unreference_proc(get_tcb(m->pid));

	size_t bytes = end - start;
	unmap_fixed_region(t->proc.vmem, start, bytes);
}

/**
 * Convenience function for freeing mapped regions.
 *
 * @param t Thread to work in.
 * @param m Memory region to free.
 */
static void __free_mapped_region(struct tcb *t, struct mem_region *m)
{
	if (m->pid != 0)
		return __free_mapped_shared_region(t, m);

	return __free_mapped_private_region(t, m);
}

void clear_uvmem(struct tcb *t)
{
	if (t->uvmem.owner != t->tid)
		return;

	struct mem_region *m = find_closest_used_region(&t->uvmem.region, 0);
	for (; m; m = m->next) {
		if (is_region_kept(m)) {
			continue;
		}

		if (!is_set(m->flags, MR_USED)) {
			continue;
		}

		__free_mapped_region(t, m);
		free_known_region(&t->uvmem.region, m);
	}
}

void purge_uvmem(struct tcb *t)
{
	if (t->uvmem.owner != t->tid)
		return;

	struct mem_region *m = find_closest_used_region(&t->uvmem.region, 0);
	for (; m; m = m->next) {
		if (!is_set(m->flags, MR_USED))
			continue;

		/* free memory associated with region */
		__free_mapped_region(t, m);
	}

	/* actually destroy region, will clear out all nodes automatically */
	destroy_region(&t->uvmem.region);
}

void destroy_uvmem(struct tcb *t)
{
	if (t->uvmem.owner != t->tid)
		return;

	/* force clear all regions */
	purge_uvmem(t);
	/* destroy associated virtual memory space */
	destroy_vmem(t->uvmem.vmem);
}

stat_t copy_uvmem(struct tcb *d, struct tcb *s)
{
	/** @todo implement some way to only iterate used regions, this loops
	 * through all regions which is likely a slight bit slower. */
	stat_t ret = OK;
	struct mem_region *m = find_first_region(&s->uvmem.region);
	while (m) {
		if (is_region_used(m))
			ret = __copy_mapped_region(d, s, m);

		if (ret)
			return ret;

		m = m->next;
	}

	return ret;
}

vm_t alloc_uvmem(struct tcb *t, size_t size, vmflags_t flags)
{
	/* t exists and is the process tcb of the current process */
	hard_assert(t && is_proc(t), ERR_INVAL);

	const vm_t v = alloc_region(&t->uvmem.region, size, &size, flags);
	if (map_region(t->proc.vmem, v, size, max_order(), flags)) {
		unmap_region(t->proc.vmem, v, size);
		free_region(&t->uvmem.region, v);
		return NULL;
	}

	return v;
}

vm_t alloc_fixed_uvmem(struct tcb *t, vm_t start, size_t size, vmflags_t flags)
{
	hard_assert(t && is_proc(t), ERR_INVAL);

	const vm_t v = alloc_fixed_region(&t->uvmem.region, start, size, &size,
	                                  flags);
	if (map_region(t->proc.vmem, v, size, max_order(), flags)) {
		unmap_region(t->proc.vmem, v, size);
		free_region(&t->uvmem.region, v);
		return NULL;
	}

	return v;
}

vm_t map_fixed_uvmem(struct tcb *t, pm_t start, size_t size, vmflags_t flags)
{
	const vm_t v = alloc_region(&t->uvmem.region, size, &size, flags);
	if (map_fixed_region(t->proc.vmem, v, start, size, flags)) {
		unmap_region(t->proc.vmem, v, size);
		free_region(&t->uvmem.region, v);
		return NULL;
	}

	return v + (start % BASE_PAGE_SIZE);
}

/* free_shared_uvmem shouldn't be needed, likely to work with free_uvmem */
vm_t alloc_shared_uvmem(struct tcb *s, size_t size, vmflags_t flags)
{
	hard_assert(s && is_proc(s), ERR_INVAL);
	const vm_t v = alloc_region(&s->uvmem.region, size, &size,
	                            MR_SHARED | flags);
	/* use base pages to make clone more likely to succeed */
	if (map_region(s->proc.vmem, v, size, BASE_PAGE, flags)) {
		unmap_region(s->proc.vmem, v, size);
		free_region(&s->uvmem.region, v);
		return NULL;
	}

	return v;
}

vm_t ref_shared_uvmem(struct tcb *d, struct tcb *s, vm_t v, vmflags_t flags)
{
	struct mem_region *m = find_used_region(&s->uvmem.region, v);
	if (!m)
		return ERR_NF;

	if (!is_set(m->flags, MR_SHARED))
		return ERR_INVAL;

	return __clone_shared_region(d, s, m, flags);
}

stat_t free_uvmem(struct tcb *r, vm_t va)
{
	/** \todo assume tcb is root tcb? */
	struct mem_region *m = find_used_region(&r->uvmem.region, va);
	if (!m)
		return ERR_NF;

	__free_mapped_region(r, m);
	free_known_region(&r->uvmem.region, m);
	return OK;
}

vmflags_t sanitize_uvflags(vmflags_t flags)
{
	return (flags & (VM_R | VM_W | VM_X)) | VM_V | VM_U;
}
