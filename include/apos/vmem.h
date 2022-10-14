/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_VMEM_H
#define APOS_VMEM_H

/**
 * @file vmem.h
 * Virtual memory handling.
 */

#include <apos/tcb.h>
#include <apos/mem.h>
#include <apos/pmem.h>
#include <apos/sp_tree.h>

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
 *
 * Only callable by servers, who are the owners of the shared region.
 *
 * @param r Process to allocate memory in.
 * @param size Minimum size of allocation.
 * @param flags Flags of allocation.
 * @return Start of allocation when succesful, \c NULL otherwise.
 */
vm_t alloc_shared_uvmem(struct tcb *r, size_t size, vmflags_t flags);

/**
 * Reference shared user virtual memory.
 *
 * Only callable by clients.
 *
 * @param r1 Process in which shared memory resides.
 * @param r2 Process to reference shared memory in.
 * @param va Virtual address of shared memory in \c r1.
 * @param flags Flags of reference in \c r2.
 * @return Start of reference in \c r2 when succesful, \c NULL otherwise.
 */
vm_t ref_shared_uvmem(struct tcb *r1, struct tcb *r2, vm_t va, vmflags_t flags);

/**
 * Free all user virtual memory allocations not marked with \ref MR_KEEP.
 *
 * @param r Process in which to clear user virtual memory.
 * @return \ref OK.
 */
stat_t clear_uvmem(struct tcb *r);

/**
 * Free all user virtual memory allocations, even if marked with \ref MR_KEEP.
 *
 * @param r Process in which to clear user virtual memory.
 * @return \ref OK.
 */
stat_t purge_uvmem(struct tcb *r);

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
 * This assumes the user virtual memory is contiguous, with no holes between \c
 * base and \c top.
 *
 * @param r Process in which to initialize user virtual memory.
 * @param base Start of user virtual memory.
 * @param top Top of user virtual memory.
 * @return \see init_region().
 */
stat_t init_uvmem(struct tcb *r, vm_t base, vm_t top);

/**
 * Destroy user virtual memory instance.
 *
 * @param r Process in which to destroy user virtual memory.
 * @return \see destroy_region().
 */
stat_t destroy_uvmem(struct tcb *r);

/**
 * User virtual memory worker callback for \ref map_fill_region().
 *
 * \c data is a pointer to \ref stat_t, which is set to \ref INFO_SEFF if all
 * threads in process should sync their memory mappings. This occurs when the
 * top level page table is modified.
 *
 * @param b Virtual memory to work in.
 * @param offset Hint for \ref alloc_page().
 * @param vaddr Current virtual address.
 * @param flags Flags of region.
 * @param order Suggested page order.
 * @param data Pointer to \ref stat_t.
 * @return \c OK when suggested order if acceptable, \c INFO_TRGN if suggested
 * order not acceptable. Error otherwise.
 * again
 */
stat_t alloc_uvmem_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                           vmflags_t flags, enum mm_order order, void *data);

/**
 * Shared user virtual memory worker callback for \ref map_fill_region().
 *
 * @param b Virtual memory to work in.
 * @param offset Hint for \ref alloc_page().
 * @param vaddr Current virtual address.
 * @param flags Flags of region.
 * @param order Suggested page order.
 * @param data Pointer to \ref stat_t.
 * @return \see alloc_uvmem_wrapper().
 *
 * \see alloc_uvmem_wrapper().
 */
stat_t alloc_shared_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                            vmflags_t flags, enum mm_order order, void *data);

/**
 * User virtual memory copying worker callback for \ref map_fill_region().
 *
 * Currently unused, but intention is to set up copy of some other virtual
 * memory region, likely passed through \c data?
 *
 * @param b Virtual memory to work in.
 * @param offset Hint for \ref alloc_page().
 * @param vaddr Current virtual address.
 * @param flags Flags of region.
 * @param order Suggested page order.
 * @param data Pointer to \ref vmem to clone from.
 * @return \see alloc_uvmem_wrapper().
 *
 * \see alloc_uvmem_wraper().
 * \todo Implement.
 */
stat_t copy_allocd_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                           vmflags_t flags, enum mm_order order, void *data);

/**
 * User virtual memory freeing worker callback for \ref map_fill_region().
 *
 * @param b Virtual memory to work in.
 * @param offset Hint for \ref alloc_page().
 * @param vaddr Current virtual address.
 * @param flags Flags of region.
 * @param order Suggested page order.
 * @param data Pointer to \ref stat_t.
 * @return \see alloc_uvmem_wrapper().
 *
 * \see alloc_uvmem_wrapper().
 */
stat_t free_uvmem_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                          vmflags_t flags, enum mm_order order, void *data);

/**
 * Convenience wrapper for \ref map_fill_region() when mapping an allocated
 * region.
 *
 * @param b Virtual memory to work in.
 * @param start Start of virtual memory region to map.
 * @param bytes Size of virtual memory region.
 * @param flags Flags of virtual memory region.
 * @param data Pointer to \c stat_t.
 * @return \see map_fill_region().
 */
#define map_allocd_region(b, start, bytes, flags, data) \
	map_fill_region(b, &alloc_uvmem_wrapper, 0, start, bytes, flags, data)

/**
 * Convenience wrapper for \ref map_fill_region() when mapping a shared region.
 *
 * @param b Virtual memory to work in.
 * @param start Start of virtual memory region to map.
 * @param bytes Size of virtual memory region.
 * @param flags Flags of virtual memory region.
 * @param data Pointer to \c stat_t.
 * @return \see map_fill_region().
 */
#define map_shared_region(b, start, bytes, flags, data) \
	map_fill_region(b, &alloc_shared_wrapper, 0, start, bytes, flags, data)

/**
 * Convenience wrapper for \ref map_fill_region() when copying a region.
 *
 * @param b Virtual memory to work in.
 * @param start Start of virtual memory region to map.
 * @param bytes Size of virtual memory region.
 * @param flags Flags of virtual memory region.
 * @param data Pointer to \c vmem to clone.
 * @return \see map_fill_region().
 */
#define copy_allocd_region(b, start, bytes, flags, data) \
	map_fill_region(b, &copy_allocd_wrapper, 0, start, bytes, flags, data)

/**
 * Convenience wrapper for \ref map_fill_region() when freeing region.
 *
 * @param b Virtual memory to work in.
 * @param start Start of virtual memory region to unmap.
 * @param bytes Size of virtual memory region.
 * @param flags Flags of virtual memory region. Technically unused?
 * @param data Pointer to \c stat_t.
 * @return \see map_fill_region().
 */
#define unmap_freed_region(b, start, bytes, flags, data) \
	map_fill_region(b, &free_uvmem_wrapper, 0, start, bytes, flags, data)

/**
 * Extract virtual memory flags (MR_XXX).
 *
 * @param x Flags to extract virtual memory region flags from.
 * @return Virtual memory region flags.
 */
#define vm_flags(x) ((x) & ~0xff)

/**
 * Extract physical memory page flags (VM_XXX).
 *
 * @param x Flags to extract physical memory page flags from.
 * @return Physical memory page flags.
 */
#define vp_flags(x) ((x) & 0xff)

#endif /* APOS_VMEM_H */
