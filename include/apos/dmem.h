#ifndef APOS_DEV_H
#define APOS_DEV_H

#include <apos/types.h>
#include <apos/vmem.h>

extern pm_t __pre_base;
extern pm_t __pre_top;
extern pm_t __post_base;
extern pm_t __post_top;

stat_t init_devmem(pm_t ram_base, pm_t ram_top);
vm_t alloc_devmem(struct tcb *t, pm_t dev_start, size_t bytes, vmflags_t flags);
stat_t free_devmem(struct tcb *t, vm_t dev_start);

stat_t dev_free_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr, vmflags_t flags, enum mm_order t);
stat_t dev_alloc_wrapper(struct vm_branch *b, pm_t *offset, vm_t vaddr, vmflags_t flags, enum mm_order t);

#endif /* APOS_DEV_H */
