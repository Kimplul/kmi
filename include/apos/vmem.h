#ifndef APOS_VMEM_H
#define APOS_VMEM_H

#include <apos/tcb.h>
#include <apos/mem.h>
#include <apos/pmem.h>
#include <apos/sp_tree.h>

vm_t alloc_uvmem(struct tcb *r, size_t size, vmflags_t flags);
vm_t alloc_fixed_uvmem(struct tcb *r, vm_t start, size_t size, vmflags_t flags);

stat_t free_uvmem(struct tcb *r, vm_t a);
stat_t init_uvmem(struct tcb *r, vm_t base, vm_t top);

stat_t alloc_uvmem_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr,
                           vmflags_t flags, enum mm_order order);
stat_t free_uvmem_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr,
                          vmflags_t flags, enum mm_order order);

#define map_allocd_region(b, start, bytes, flags)                              \
	map_fill_region(b, &alloc_uvmem_wrapper, 0, start, bytes, flags)

#define unmap_freed_region(b, start, bytes)                                    \
	map_fill_region(b, &free_uvmem_wrapper, 0, start, bytes, 0)

#define vm_flags(x) ((x) & ~0xff)
#define vp_flags(x) ((x)&0xff)

#endif /* APOS_VMEM_H */
