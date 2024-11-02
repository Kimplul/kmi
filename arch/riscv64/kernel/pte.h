#ifndef KMI_RISCV_PTE_H
#define KMI_RISCV_PTE_H

/**
 * Get page table entry physical page number.
 *
 * @param pte Page table entry.
 * @return Corresponding physical page number.
 */
#define pte_ppn(pte) (((pm_t)(pte)) >> 10)

/**
 * Get page table entry flags.
 *
 * @param pte Page table entry.
 * @return Corresponding flags.
 */
#define pte_flags(pte) (((pm_t)(pte)) & 0xff)

/**
 * Convert physical memory address to page table entry.
 *
 * @param p Physical memory address.
 * @param f Flags to use.
 * @return Corresponding page table entry.
 */
#define to_pte(p, f) ((((p) >> page_shift()) << 10) | (f))

/**
 * Get physical address in page table entry.
 *
 * @param pte Page table entry.
 * @return Corresponding physical address.
 */
#define pte_paddr(pte) (pte_ppn(pte) << page_shift())

/**
 * Get virtual address in page table entry.
 *
 * @param pte Page table entry.
 * @return Corresponding virtual address.
 */
#define pte_addr(pte) __va(pte_paddr(pte))

/**
 * Virtual memory address to page order index.
 *
 * @param a Virtual address.
 * @param o Order of page.
 * @return Corresponding page index.
 */
#define vm_to_index(a, o) (pm_to_index(a, o))

/**
 * Check if page table entry is active.
 *
 * @param pte Page table entry.
 * @return \c 0 if entry is not active, non-zero otherwise.
 */
#define is_active(pte) (pte_flags(pte) &VM_V)

/**
 * Check if page table entry is a leaf.
 *
 * @param pte Page table entry.
 * @return \c 0 if entry is not leaf, non-zero otherwise.
 */
#define is_leaf(pte) (is_active(pte) && (pte_flags(pte) & ~VM_V))

/**
 * Check if page table entry is a branch.
 *
 * @param pte Page table entry.
 * @return \c 0 if entry is not branch, non-zero otherwise.
 */
#define is_branch(pte) (is_active(pte) && !(pte_flags(pte) & ~VM_V))

#endif /* KMI_RISCV_PTE_H */
