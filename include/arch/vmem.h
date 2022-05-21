#ifndef APOS_ARCH_PAGES_H
#define APOS_ARCH_PAGES_H

#include <vmem.h>

stat_t map_vpage(struct vm_branch *branch, pm_t paddr, vm_t vaddr,
                 vmflags_t flags, enum mm_order order);

stat_t unmap_vpage(struct vm_branch *branch, vm_t vaddr);

stat_t mod_vpage(struct vm_branch *branch, vm_t vaddr, pm_t paddr,
                 vmflags_t flags);
stat_t stat_vpage(struct vm_branch *branch, vm_t vaddr, pm_t *paddr,
                  enum mm_order *order, vmflags_t *flags);

void flush_tlb();
void flush_tlb_all();

void populate_root_branch(struct vm_branch *b);
struct vm_branch *init_vmem(void *fdt);

#if defined(DEBUG)
vm_t setup_kernel_io(struct vm_branch *b, vm_t paddr);
#endif

stat_t clone_vmbranch(struct vm_branch *, struct vm_branch *);

#endif /* APOS_ARCH_PAGES_H */
