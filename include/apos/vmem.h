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

vm_t alloc_uvmem(struct tcb *r, size_t size, vmflags_t flags);
vm_t alloc_fixed_uvmem(struct tcb *r, vm_t start, size_t size, vmflags_t flags);
vm_t alloc_shared_uvmem(struct tcb *r, size_t size, vmflags_t flags);
vm_t ref_shared_uvmem(struct tcb *r1, struct tcb *r2, vm_t va, vmflags_t flags);

stat_t clear_uvmem(struct tcb *r, bool force);
stat_t free_uvmem(struct tcb *r, vm_t a);

stat_t init_uvmem(struct tcb *r, vm_t base, vm_t top);
stat_t destroy_uvmem(struct tcb *r);

stat_t alloc_uvmem_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                           vmflags_t flags, enum mm_order order, void *data);
stat_t alloc_shared_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                            vmflags_t flags, enum mm_order order, void *data);
stat_t clone_allocd_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                            vmflags_t flags, enum mm_order order, void *data);
stat_t free_uvmem_wrapper(struct vmem *b, pm_t *offset, vm_t vaddr,
                          vmflags_t flags, enum mm_order order, void *data);

#define map_allocd_region(b, start, bytes, flags, data)                        \
	map_fill_region(b, &alloc_uvmem_wrapper, 0, start, bytes, flags, data)

#define map_shared_region(b, start, bytes, flags, data)                        \
	map_fill_region(b, &alloc_shared_wrapper, 0, start, bytes, flags, data)

#define clone_allocd_region(b, start, bytes, flags, data) \
	map_fill_region(b, &clone_allocd_wrapper, 0, start, bytes, flags, data)

#define unmap_freed_region(b, start, bytes, flags, data)                       \
	map_fill_region(b, &free_uvmem_wrapper, 0, start, bytes, flags, data)

#define vm_flags(x) ((x) & ~0xff)
#define vp_flags(x) ((x)&0xff)

#endif /* APOS_VMEM_H */
