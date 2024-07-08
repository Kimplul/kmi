/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_ARCH_PAGES_H
#define KMI_ARCH_PAGES_H

/**
 * @file vmem.h
 * Arch-specific virtual memory handling, generally implemented in
 * arch/whatever/kernel/vmem.c
 */

#if defined(__riscv)
# if __riscv_xlen == 64
#include "../../arch/riscv64/include/vmem.h"
# else
#include "../../arch/riscv32/include/vmem.h"
# endif
#endif

#include <kmi/types.h>
#include <kmi/attrs.h>

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
 * Set flags in page at virtual address \p vaddr.
 *
 * @param branch Branch in which to work.
 * @param vaddr Virtual address of page.
 * @param flags Flags to set.
 * @return \ref ERR_NF if no page could be found at \p vaddr,
 *	\ref INFO_SEFF if modification has side effects, otherwise \ref OK.
 */
stat_t set_vpage_flags(struct vmem *branch, vm_t vaddr, vmflags_t flags);

/**
 * Clear flags in page at virtual address \p vaddr.
 *
 * @param branch Branch in which to work.
 * @param vaddr Virtual address of page.
 * @param flags Flags to clear.
 * @return \ref ERR_NF if no page could be found at \p vaddr,
 *	\ref INFO_SEFF if modification has side effects, otherwise \ref OK.
 */
stat_t clear_vpage_flags(struct vmem *branch, vm_t vaddr, vmflags_t flags);

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

/**
 * Flush tlb entry where associated with address \p addr.
 *
 * @param addr Virtual address region to flush.
 */
void flush_tlb(uintptr_t addr);

/** Flush full tlb of current address space. */
void flush_tlb_full();

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
 * Create a direct mapping, with 1:1 physical addresses in one half of the
 * address space and the kernel address space in the other.
 * Called at boot, not allowed to allocate memory.
 *
 * @param load_addr Where in physical memory kernel was loaded to. Could in
 * theory be fetched from \ref get_load_addr(), but the fewer globals are
 * touched during initialization the better.
 *
 * @return The vmem node used to build the address space. Probably statically
 * allocated.
 */
struct vmem *init_mapping(uintptr_t load_addr);

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
 */
void use_vmem(struct vmem *b);

/**
 * Destroy virtual memory space.
 *
 * @param b Virtual memory to destroy.
 */
void destroy_vmem(struct vmem *b);

/**
 * Raw clone user virtual memory.
 *
 * @param r Source virtual memory of clone.
 * @param b Destination virtual memory of clone.
 */
void clone_uvmem(struct vmem * restrict r, struct vmem * restrict b);

/**
 * Jump into kernelspace from a physical address space.
 * Really only called by main() during boot so the parameters are kind of weird.
 *
 * @param fdt Flattened devicetree.
 * @param load_addr Address where kernel was loaded to.
 * @param direct_mapping Direct mapping vmem.
 * @param ram_base Physical base address of ram.
 */
__noreturn void to_kernelspace(void *fdt,
                               uintptr_t load_addr,
                               struct vmem *direct_mapping,
                               pm_t ram_base);
#endif /* KMI_ARCH_PAGES_H */
