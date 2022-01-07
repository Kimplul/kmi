#ifndef APOS_ARCH_PAGES_H
#define APOS_ARCH_PAGES_H

#include <vmem.h>

void map_vmem(struct vm_branch *branch,
		pm_t paddr, vm_t vaddr, uint8_t flags, enum mm_order order);

void unmap_vmem(struct vm_branch *branch, vm_t vaddr);

int mod_vmem(struct vm_branch *branch, vm_t vaddr, pm_t paddr, uint8_t flags);
int stat_vmem(struct vm_branch *branch, vm_t vaddr, pm_t *paddr,
		enum mm_order *order, uint8_t *flags);

void flush_tlb();
void flush_tlb_all();

void populate_root_branch(struct vm_branch *b);
int setup_kernel_io(struct vm_branch *b, vm_t paddr);
struct vm_branch *init_vmem(void *fdt);

#endif /* APOS_ARCH_PAGES_H */
