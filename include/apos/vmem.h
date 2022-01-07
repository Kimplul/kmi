#ifndef APOS_VMEM_H
#define APOS_VMEM_H

#include <apos/tcb.h>
#include <apos/pmem.h>
#include <apos/sp_tree.h>
#include <arch/vmem.h>

vm_t alloc_uvmem(struct tcb *r, size_t size, uint8_t flags);
vm_t alloc_fixed_uvmem(struct tcb *r, vm_t start, size_t size, uint8_t flags);

stat_t free_uvmem(struct tcb *r, vm_t a);
stat_t init_uvmem(struct tcb *r, vm_t base, vm_t top);

vm_t map_fill_region(struct vm_branch *b,
		int (*vmem_handler)(struct vm_branch *, pm_t *, vm_t, uint8_t, enum mm_order),
		pm_t offset, vm_t start, size_t bytes, uint8_t flags);

stat_t alloc_uvmem_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr, uint8_t flags, enum mm_order order);
stat_t free_uvmem_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr, uint8_t flags, enum mm_order order);

#define map_allocd_region(b, start, bytes, flags)\
	map_fill_region(b, &alloc_uvmem_wrapper, 0, start, bytes, flags)

#define unmap_freed_region(b, start, bytes)\
	map_fill_region(b, &free_uvmem_wrapper, 0, start, bytes, 0)

size_t uvmem_size();
void set_uvmem_size(size_t s);

#endif /* APOS_VMEM_H */
