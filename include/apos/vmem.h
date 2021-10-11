#ifndef APOS_VMEM_H
#define APOS_VMEM_H

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

void map_vmem(struct vm_branch_t *branch,
		pm_t paddr, vm_t vaddr,
		uint8_t flags, enum mm_order_t order);

void unmap_vmem(struct vm_branch_t *branch, vm_t vaddr, enum mm_order_t order);

#define map_kvregion(b, pb, pt) map_vregion(b, pb, pt, VMEM_BASE, VMEM_TOP);
#define map_uvregion(b, pb, pt) map_vregion(b, pb, pt, UMEM_BASE, UMEM_TOP);

vm_t map_vregion(struct vm_branch_t *branch, pm_t base, pm_t top, vm_t start, vm_t end);
void unmap_vregion(struct vm_branch_t *branch, pm_t base, pm_t top);

vm_t map_vsize(struct vm_branch_t *branch, size_t size);
void unmap_vsize(struct vm_branch_t *branch, size_t size);

#endif /* APOS_VMEM_H */
