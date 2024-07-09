/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_VMEM_H
#define KMI_VMEM_H

/**
 * @file vmem.h
 * Virtual memory handling.
 */

#include <kmi/tcb.h>
#include <kmi/mem.h>
#include <kmi/pmem.h>
#include <kmi/regions.h>
#include <kmi/sp_tree.h>

/**
 * Allocate user virtual memory.
 *
 * Virtual memory start address is chosen according to best fit with regard to
 * size.
 *
 * @param r Process to allocate memory in.
 * @param size Minimum size of allocation.
 * @param flags Flags of allocation.
 * @return Start of allocation when succesful, \c NULL otherwise.
 */
vm_t alloc_uvmem(struct tcb *r, size_t size, vmflags_t flags);

/**
 * Allocate one physical page for user virtual memory.
 * Useful for virtio buffers etc. I'm repeating myself quite a lot with these
 * doxygen descriptions, aren't I?
 *
 * @param r Process to allocate memory in.
 * @param size Minimum size of allocation.
 * @param flags Flags of allocation.
 * @param startp Address of allocated physical page.
 * @param sizep Actual size of page.
 * @return Start of allocation when succesful, \c NULL otherwise.
 */
vm_t alloc_uvpage(struct tcb *r, size_t size, vmflags_t flags,
                  pm_t *startp, size_t *sizep);

/**
 * Allocate fixed user virtual memory.
 *
 * Virtual memory start address is chosen so that \c start is within the
 * allocation and the allocation after \c start is at least \c size bytes large.
 * It is unspecified how many bytes are between the start of the allocation and
 * \c start.
 *
 * @param r Process to allocate memory in.
 * @param start Address that should be in allocation.
 * @param size Minimum size of allocation.
 * @param flags Flags of allocation.
 * @return Start of allocation when succesful, \c NULL otherwise.
 */
vm_t alloc_fixed_uvmem(struct tcb *r, vm_t start, size_t size, vmflags_t flags);

/**
 * Allocate shared user virtual memory.
 * Initially this region is only visible to whoever allocated it, but calling
 * \ref ref_shared_uvmem() with the address of this allocation
 * will add mappings to this region into other processes' address spaces.
 *
 * @param s First process to allocate memory in.
 * @param size Minimum size of allocation.
 * @param flags Flags of allocation.
 * @return Address of allocation.
 */
vm_t alloc_shared_uvmem(struct tcb *s, size_t size, vmflags_t flags);

/**
 * Free all user virtual memory allocations not marked with \ref MR_KEEP.
 *
 * @param r Process in which to clear user virtual memory.
 */
void clear_uvmem(struct tcb *r);

/**
 * Free all user virtual memory allocations, even if marked with \ref MR_KEEP.
 *
 * @param r Process in which to clear user virtual memory.
 */
void purge_uvmem(struct tcb *r);

/**
 * Free one user virtual memory allocation.
 *
 * @param r Process in which to clear user virtual memory.
 * @param va Start of user virtual memory allocation to free.
 * @return \ref OK.
 */
stat_t free_uvmem(struct tcb *r, vm_t va);

/**
 * Initialize user virtual memory instance.
 *
 * This assumes the user virtual memory is contiguous, with no holes between
 * \ref UVMEM_START and \ref UVMEM_END. One base page at the start of the region
 * is reserved for use as a NULL page, which can be freed by the user to
 * 'accept' that it's dangerous to not have a NULL page.
 *
 * Requires that \p t already has a vmem allocated.
 *
 * @param r Process in which to initialize user virtual memory.
 * @return \see init_region().
 */
stat_t init_uvmem(struct tcb *r);

/**
 * Destroy user virtual memory instance.
 *
 * @param r Process in which to destroy user virtual memory.
 */
void destroy_uvmem(struct tcb *r);

/**
 * Map a fixed physical region (within kernelspace) to somewhere in virtual
 * memory.
 *
 * @param r Thread where memory should be mapped.
 * @param base Physical base address to map.
 * @param size Size of region to map.
 * @param flags Flags to use for mapping.
 * @return Address of mapping in virtual memory. Though note that it points to
 * the start of \p base, not necessarily the start of the allocation.
 * If this should be freed, remember to align down to the base page size.
 */
vm_t map_fixed_uvmem(struct tcb *r, pm_t base, size_t size, vmflags_t flags);

/**
 * Clone process memory.
 *
 * @param d Destination tcb.
 * @param s Source tcb.
 * @return OK.
 *
 * @todo Come up with better name. clone_uvmem() is taken, but should it be
 * renamed to clone_mapping() or something?
 */
stat_t clone_mem_regions(struct tcb *d, struct tcb *s);

/**
 * Reference a shared memory region.
 * Adds a mapping in \p d of region \p v in \p s.
 *
 * @param d For which thread to create a new mapping.
 * @param s Owner of shared region.
 * @param v Address of shared region in \p s.
 * @param flags Flags for mapping in \p d.
 * @return Address of mapping in \p d.
 */
vm_t ref_shared_uvmem(struct tcb *d, struct tcb *s, vm_t v, vmflags_t flags);

/**
 * Create a copy of \p s in \p d. Used for implementing \ref fork().
 *
 * @param d Destination of copy.
 * @param s Source of copy.
 * @return \ref OK on success, some error otherwise.
 */
stat_t copy_uvmem(struct tcb *d, struct tcb *s);

/**
 * Clears out all other flags besides VM_W/VM_R/VM_X from user-provided flags
 * and adds VM_U and VM_V to make mapping accessible from userspace.
 *
 * @param flags Flags to sanitize.
 * @return Sanitized flags.
 */
vmflags_t sanitize_uvflags(vmflags_t flags);

#endif /* KMI_VMEM_H */
