#ifndef APOS_DEV_H
#define APOS_DEV_H

#include <apos/types.h>
#include <apos/vmem.h>

extern pm_t __pre_base;
extern pm_t __pre_top;
extern pm_t __post_base;
extern pm_t __post_top;

void init_devmem(pm_t ram_base, pm_t ram_top);
vm_t alloc_devmem(struct tcb *t, pm_t dev_start, size_t bytes, uint8_t flags);
void free_devmem(struct tcb *t, vm_t dev_start);

#endif /* APOS_DEV_H */
