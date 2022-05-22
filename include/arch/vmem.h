#ifndef APOS_ARCH_PAGES_H
#define APOS_ARCH_PAGES_H

/**
 * @file vmem.h
 * Arch-specific virtual memory handling, generally implemented in
 * arch/whatever/kernel/vmem.c
 */

#include <vmem.h>

stat_t map_vpage(struct vmem *branch, pm_t paddr, vm_t vaddr, vmflags_t flags,
                 enum mm_order order);

stat_t unmap_vpage(struct vmem *branch, vm_t vaddr);

stat_t mod_vpage(struct vmem *branch, vm_t vaddr, pm_t paddr, vmflags_t flags);
stat_t stat_vpage(struct vmem *branch, vm_t vaddr, pm_t *paddr,
                  enum mm_order *order, vmflags_t *flags);

void flush_tlb();
void flush_tlb_all();

stat_t populate_kvmem(struct vmem *b);
struct vmem *init_vmem(void *fdt);

#if defined(DEBUG)
vm_t setup_kernel_io(struct vmem *b, vm_t paddr);
#endif

struct vmem *create_vmem();
stat_t destroy_vmem(struct vmem *);
stat_t clone_uvmem(struct vmem *, struct vmem *);

#endif /* APOS_ARCH_PAGES_H */
