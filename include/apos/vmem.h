#ifndef APOS_VMEM_H
#define APOS_VMEM_H

#include <apos/pmem.h>

/* arch-specific data */
#include <vmem.h>

/* general overview of the different functions:
 * (un)map_vmem: map one known page of physical memory to one known page of
 * virtual memory
 *
 * (un)map_vregion: map known physical region to unknown virtual region within
 * start and end
 *
 * (un)map_vsize: map unknown physical region to unknown virtual region
 */

/* defined by arch */
void map_vmem(struct vm_branch_t *branch,
		pm_t paddr, vm_t vaddr,
		uint8_t flags, enum mm_order_t order);

void unmap_vmem(struct vm_branch_t *branch, vm_t vaddr, enum mm_order_t order);


vm_t map_vregion(struct vm_branch_t *branch, pm_t base, vm_t start, size_t size,
		uint8_t flags);
void unmap_vregion(struct vm_branch_t *branch, vm_t start);

#endif /* APOS_VMEM_H */
