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

stat_t init_uvmem(struct tcb *t)
{
	t->uvmem.owner = t->tid;
	t->uvmem.vmem = t->proc.vmem;
	/* reserve 64KiB (arbitrary number but should be large enough that
	 * nobody accidentally indexes above NULL enough to find real memory) */
	return init_region(&t->uvmem.region, UVMEM_START,
	                   UVMEM_END - UVMEM_START, SZ_64K);
}

/**
 * Copy process memory region.
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
	vm_t start = m->start * BASE_PAGE_SIZE;
	vm_t end = m->end * BASE_PAGE_SIZE;

	size_t size = end - start;
	vm_t v = alloc_fixed_region(&d->uvmem.region, start, size, &size,
	                            m->flags);
	if (ERR_CODE(v))
		return v;

	assert(v == start);

	if (is_set(m->flags, MR_NONBACKED))
		return v;

	/* note that we use uvmem.vmem instead of proc.vmem, this is just to
	 * make sure that zombies don't eat our brains */
	stat_t res = copy_region(d->uvmem.vmem, s->uvmem.vmem, v, v, size);
	if (res == OK)
		return OK;

	/* cleanup on error */
	free_region(&d->uvmem.region, v);
	unmap_region(d->uvmem.vmem, v, size);
	return res;
}

/**
 * Copy shared regions to new process. In these cases, we want to both allocate
 * a fixed region and map some fixed physical memory.
 *
 * @param d 'Destination'
 * @param m Shared memory region.
 * @return \ref OK on success, some error code otherwise.
 */
static stat_t __copy_shared_region(struct tcb *d, struct mem_region *m)
{
	struct tcb *s = get_tcb(m->pid);
	if (!s)
		return ERR_NF;

	vm_t start = m->start * BASE_PAGE_SIZE;
	vm_t end = m->end * BASE_PAGE_SIZE;

	reference_thread(s);

	size_t size = end - start;
	vm_t v = alloc_fixed_region(&d->uvmem.region, start, size, &size,
	                            m->flags);
	if (ERR_CODE(v))
		return v;

	assert(v == start);
	stat_t res = clone_region(d->uvmem.vmem, s->uvmem.vmem, start, v, size,
	                          m->flags);
	if (res == OK)
		return OK;

	/* cleanup on error */
	free_region(&d->uvmem.region, v);
	unmap_fixed_region(d->uvmem.vmem, v, size);
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

	reference_thread(s);

	size_t size = end - start;
	vm_t v = alloc_shared_region(&d->uvmem.region, size, &size,
	                             MR_NONBACKED | m->flags, s->rid);
	if (ERR_CODE(v))
		return v;

	stat_t res = clone_region(d->uvmem.vmem, s->uvmem.vmem,
	                          start, v, size, flags);
	if (res == OK)
		return v;

	/* cleanup on error */
	unreference_thread(s);
	free_region(&d->uvmem.region, v);
	unmap_fixed_region(d->uvmem.vmem, v, size);
	return res;
}

/**
 * Unmap and free private memory region.
 *
 * @param t Current thread.
 * @param m Memory region to free.
 */
static void __free_mapping(struct tcb *t, struct mem_region *m)
{
	struct tcb *owner = get_tcb(m->pid);
	if (owner)
		unreference_thread(owner);

	if (is_set(m->flags, MR_NONBACKED))
		return;

	pm_t start = __addr(m->start);
	pm_t end = __addr(m->end);
	size_t size = end - start;

	if (m->pid)
		unmap_fixed_region(t->proc.vmem, start, size);
	else
		unmap_region(t->proc.vmem, start, size);
}

void clear_uvmem(struct tcb *t)
{
	if (t->uvmem.owner != t->tid)
		return;

	struct mem_region *m = find_closest_used_region(&t->uvmem.region, 0);
	for (; m; m = m->next) {
		if (is_region_kept(m))
			continue;

		if (!is_set(m->flags, MR_USED))
			continue;

		__free_mapping(t, m);
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

		__free_mapping(t, m);
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
	for (; m; m = m->next) {
		if (!is_region_used(m))
			continue;

		if (m->pid == 0)
			ret = __copy_mapped_region(d, s, m);
		else
			ret = __copy_shared_region(d, m);

		if (ret)
			return ret;
	}

	return ret;
}

vm_t alloc_uvmem(struct tcb *t, size_t size, vmflags_t flags)
{
	/* t exists and is the process tcb of the current process */
	assert(t && is_proc(t));

	const vm_t v = alloc_region(&t->uvmem.region, size, &size, flags);
	if (ERR_CODE(v))
		return v;

	stat_t ret = OK;
	if ((ret = map_region(t->proc.vmem, v, size, max_order(), flags))) {
		unmap_region(t->proc.vmem, v, size);
		free_region(&t->uvmem.region, v);
		return ret;
	}

	return v;
}

vm_t alloc_fixed_uvmem(struct tcb *t, vm_t start, size_t size, vmflags_t flags)
{
	assert(t && is_proc(t));

	const vm_t v = alloc_fixed_region(&t->uvmem.region, start, size, &size,
	                                  flags);
	if (ERR_CODE(v))
		return v;

	stat_t ret = OK;
	if ((ret = map_region(t->proc.vmem, v, size, max_order(), flags))) {
		unmap_region(t->proc.vmem, v, size);
		free_region(&t->uvmem.region, v);
		return ret;
	}

	return v;
}

vm_t map_fixed_uvmem(struct tcb *t, pm_t start, size_t size, vmflags_t flags)
{
	assert(is_aligned(start, BASE_PAGE_SIZE));

	const vm_t v = alloc_region(&t->uvmem.region, size, &size, flags);
	if (ERR_CODE(v))
		return v;

	stat_t ret = OK;
	if ((ret = map_fixed_region(t->proc.vmem, v, start, size, flags))) {
		unmap_region(t->proc.vmem, v, size);
		free_region(&t->uvmem.region, v);
		return ret;
	}

	return v;
}

vm_t alloc_uvpage(struct tcb *t, size_t size, vmflags_t flags, pm_t *startp,
                  size_t *sizep)
{
	enum mm_order order = nearest_order(size);
	size = order_size(order);

	const vm_t v = alloc_region(&t->uvmem.region, size, &size, flags);
	if (ERR_CODE(v))
		return v;

	pm_t start = alloc_page(order);
	if (!start) {
		free_region(&t->uvmem.region, size);
		return ERR_OOMEM;
	}

	stat_t ret = OK;
	if ((ret = map_fixed_region(t->proc.vmem, v, start, size, flags))) {
		unmap_region(t->proc.vmem, v, size);
		free_region(&t->uvmem.region, v);
		return ret;
	}

	*startp = (pm_t)__pa(start);
	*sizep = size;
	return v;
}


/* free_shared_uvmem shouldn't be needed, likely to work with free_uvmem */
vm_t alloc_shared_uvmem(struct tcb *s, size_t size, vmflags_t flags)
{
	assert(s && is_proc(s));
	const vm_t v = alloc_region(&s->uvmem.region, size, &size,
	                            MR_SHARED | flags);
	if (ERR_CODE(v))
		return v;

	/* use base pages to make clone more likely to succeed */
	stat_t ret = OK;
	if ((ret = map_region(s->proc.vmem, v, size, BASE_PAGE, flags))) {
		unmap_region(s->proc.vmem, v, size);
		free_region(&s->uvmem.region, v);
		return ret;
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

	__free_mapping(r, m);
	free_known_region(&r->uvmem.region, m);
	return OK;
}

vmflags_t sanitize_uvflags(vmflags_t flags)
{
	return (flags & (VM_R | VM_W | VM_X)) | VM_V | VM_U;
}
