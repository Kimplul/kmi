#ifndef APOS_ARCH_PAGES_H
#define APOS_ARCH_PAGES_H

/**
 * @file vmem.h
 * Arch-specific virtual memory handling, generally implemented in
 * arch/whatever/kernel/vmem.c
 */

#if defined(riscv64)
#include "../../arch/riscv64/include/vmem.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/vmem.h"
#endif

/**
 * Map one virtual page to physical page.
 *
 * If \ref INFO_SEFF is returned, the caller is responsible for synchronizing
 * all other threads that are in the same virtual address space.
 *
 * The caller is responsible for checking that both virtual and physical page of
 * the correct order are available.
 *
 * @param branch Virtual memory to work in.
 * @param paddr Physical address of page.
 * @param vaddr Virtual address to map page to.
 * @param flags Page flags.
 * @param order Order of page.
 * @return \ref INFO_SEFF when top level table modified, \ref OK otherwise.
 */
stat_t map_vpage(struct vmem *branch, pm_t paddr, vm_t vaddr, vmflags_t flags,
                 enum mm_order order);

/**
 * Unmap one page.
 *
 * @param branch Virtual memory to work in.
 * @param vaddr Virtual address of page to unmap.
 * @return \ref OK when succesful, \ref ERR_NF if no page at \c vaddr could be
 * found.
 */
stat_t unmap_vpage(struct vmem *branch, vm_t vaddr);

/**
 * Modify page mapping physical address and flags.
 *
 * Note that changing the order of a page is not supported, to do that unmap and
 * remap a page.
 *
 * @param branch Virtual memory to work in.
 * @param vaddr Virtual address of map to modify.
 * @param paddr Physical address to map to.
 * @param flags Flags to set.
 * @return \ref ERR_NF if no virtual page is found at \c vaddr.
 * \ref INFO_SEFF if modding takes place in top page table but otherwise
 * succesful, \ref OK otherwise.
 */
stat_t mod_vpage(struct vmem *branch, vm_t vaddr, pm_t paddr, vmflags_t flags);

/**
 * Get information about physical page.
 *
 * @param branch Virtual memory to work in.
 * @param vaddr Virtual address of map to get parameters of.
 * @param paddr Where to write physical address of the page.
 * @param order Where to write the order of the page.
 * @param flags Where to write the physical mapping flags of the page.
 * @return \ref ERR_NF when no page is found at \c vaddr, \ref OK otherwise.
 */
stat_t stat_vpage(struct vmem *branch, vm_t vaddr, pm_t *paddr,
                  enum mm_order *order, vmflags_t *flags);

/** Flush tlb of current processor. */
void flush_tlb();

/** Flush tlbs of all processors. */
void flush_tlb_all();

/** \todo Add flushing of only some region in memory? */

/**
 * Populate virtual memory with kernel virtual address info.
 *
 * @param b Virtual memory to work in.
 * @return \ref OK.
 */
stat_t populate_kvmem(struct vmem *b);

/**
 * Initialize arch virtual memory and return boot virtual memory.
 *
 * @param fdt Global FDT pointer.
 * @return Pointer to new boot virtual memory when succesful, \c NULL otherwise.
 */
struct vmem *init_vmem(void *fdt);

#if defined(DEBUG)
/**
 * Map and setup serial port.
 *
 * @param b Virtual memory to work in.
 * @param paddr Physical address of serial port.
 * @return Virtual address of serial port.
 */
vm_t setup_kernel_io(struct vmem *b, vm_t paddr);
#endif

/**
 * Create new virtual memory space.
 *
 * @return Pointer to virtual memory root when succesful, \c NULL otherwise.
 */
struct vmem *create_vmem();

/**
 * Jump into virtual memory context.
 *
 * @param b Virtual memory to jump into.
 * @return \ref OK.
 */
stat_t use_vmem(struct vmem *b);

/**
 * Destroy virtual memory space.
 *
 * @param b Virtual memory to destroy.
 * @return \ref OK.
 */
stat_t destroy_vmem(struct vmem *b);

/**
 * Raw clone user virtual memory.
 *
 * @param r Source virtual memory of clone.
 * @param b Destination virtual memory of clone.
 * @return \ref OK.
 */
stat_t clone_uvmem(struct vmem *r, struct vmem *b);

#endif /* APOS_ARCH_PAGES_H */
