/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file vmem.c
 * riscv64 implementation of arch-specific virtual memory handling.
 */

#include <apos/string.h>
#include <apos/pmem.h>
#include <apos/vmem.h>
#include <apos/mem.h>
#include <apos/debug.h>
#include <arch/cpu.h>
#include "pages.h"
#include "csr.h"

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
#define to_pte(p, f) ((pm_to_pnum(p) << 10) | (f))

/**
 * Get virtual address in page table entry.
 *
 * @param pte Page table entry.
 * @return Corresponding virtual address.
 */
#define pte_addr(pte) __va(pnum_to_pm(pte_ppn(pte)))

/**
 * Get physical address in page table entry.
 *
 * @param pte Page table entry.
 * @return Corresponding physical address.
 */
#define pte_paddr(pte) (pnum_to_pm(pte_ppn(pte)))

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
#define is_active(pte) (pte_flags(pte) & VM_V)

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

/**
 * Find page table entry corresponding to virtual address.
 *
 * @param b Virtual memory to work in.
 * @param v Virtual address to look for.
 * @param o Address where to return page order to.
 * @return Physical address of page.
 */
static pm_t *__find_vmem(struct vmem *b, vm_t v, enum mm_order *o)
{
	enum mm_order top = __mm_max_order;
	if (o)
		*o = MM_O0;
	do {
		size_t idx = vm_to_index(v, top);
		pm_t pte = (pm_t)b->leaf[idx];

		if (!pte)
			return 0;

		if (is_leaf(pte)) {
			if (o)
				*o = top;

			return (pm_t *)&b->leaf[idx];
		}

		b = (struct vmem *)pte_addr(pte);
	} while (top--);

	return 0;
}

stat_t mod_vpage(struct vmem *branch, vm_t vaddr, pm_t paddr, vmflags_t flags)
{
	enum mm_order order;
	pm_t *pte = __find_vmem(branch, vaddr, &order);
	if (pte) {
		*pte = to_pte((pm_t)__pa(paddr), vp_flags(flags));
		/* if we're modifying a top level mapping, we will have to
		 * update the same one for all the other threads in this process
		 * */
		if (order == __mm_max_order)
			return INFO_SEFF;
		else
			return OK;
	}

	return ERR_NF;
}

/* huh, should probably add status flags etc. to all my API functions. Damn, I'm
 * lazy. */
stat_t stat_vpage(struct vmem *branch, vm_t vaddr, pm_t *paddr,
                  enum mm_order *order, vmflags_t *flags)
{
	pm_t *pte = __find_vmem(branch, vaddr, order);
	if (pte) {
		if (paddr)
			*paddr = (pm_t)pte_addr(*pte);

		if (flags)
			*flags = pte_flags(*pte);

		return OK;
	}

	return ERR_NF;
}

/**
 * Create virtual memory leaf page table.
 *
 * @return New virtual memory leaf page table.
 */
static struct vmem *__create_leaf()
{
	pm_t new_leaf = alloc_page(MM_KPAGE, 0);
	memset((void *)new_leaf, 0, sizeof(struct vmem));
	return (struct vmem *)to_pte((pm_t)__pa(new_leaf), VM_V);
}

/**
 * Destroy virtual memory page table branch.
 *
 * @param b Virtual memory to work in.
 */
static void __destroy_branch(struct vmem *b)
{
	if (!b)
		return;

	for (size_t i = 0; i < RISCV_NUM_LEAVES; ++i) {
		if (is_branch(b->leaf[i]))
			__destroy_branch((struct vmem *)pte_addr(b->leaf[i]));
	}

	free_page(MM_KPAGE, (pm_t)__pa(b));
}

stat_t map_vpage(struct vmem *branch, pm_t paddr, vm_t vaddr, vmflags_t flags,
                 enum mm_order order)
{
	enum mm_order top = __mm_max_order;
	while (top != order) {
		size_t idx = vm_to_index(vaddr, top);

		if (!branch->leaf[idx])
			branch->leaf[idx] = __create_leaf();

		branch = (struct vmem *)pte_addr(branch->leaf[idx]);
		top--;
	}

	size_t idx = vm_to_index(vaddr, top);
	if (is_branch(
		    branch->leaf[idx])) /* something has gone terribly wrong? */
		__destroy_branch(branch->leaf[idx]);

	branch->leaf[idx] =
		(struct vmem *)to_pte((pm_t)__pa(paddr), vp_flags(flags));

	return top == __mm_max_order ? INFO_SEFF : OK;
}

stat_t unmap_vpage(struct vmem *branch, vm_t vaddr)
{
	pm_t *pte = __find_vmem(branch, vaddr, 0);
	if (pte) {
		*pte = 0;
		return OK;
	}

	return ERR_NF;
}

void flush_tlb()
{
	__asm__ volatile ("sfence.vma %0\n" : : "r" (cpu_id()) : "memory");
}

void flush_tlb_all()
{
	__asm__ volatile ("sfence.vma\n" ::: "memory");
}

/**
 * Jump into virtual memory.
 *
 * @param branch Virtual memory address space to jump into.
 * @param m Riscv memory mode to use.
 */
static void __use_vmem(struct vmem *branch, enum mm_mode m)
{
	branch = (struct vmem *)__pa(branch);

	if (m == Sv32)
		csr_write(CSR_SATP,
		          SATP_MODE_Sv32 | pm_to_pnum((pm_t)(branch)));
	else if (m == Sv39)
		csr_write(CSR_SATP,
		          SATP_MODE_Sv39 | pm_to_pnum((pm_t)(branch)));
	else
		csr_write(CSR_SATP,
		          SATP_MODE_Sv48 | pm_to_pnum((pm_t)(branch)));

	flush_tlb_all();
	/* Sv57 && Sv64 in the future? */
}

struct vmem *init_vmem(void *fdt)
{
	UNUSED(fdt);
	struct vmem *b = create_vmem();
	/* update which memory branch to use */
	use_vmem(b);
	return b;
}

struct vmem *create_vmem()
{
	struct vmem *b = (struct vmem *)alloc_page(MM_KPAGE, 0);
	memset(b, 0, MM_KPAGE_SIZE);

	populate_kvmem(b);
	return b;
}

stat_t use_vmem(struct vmem *b)
{
	__use_vmem(b, Sv39);
	return OK;
}

stat_t destroy_vmem(struct vmem *b)
{
	__destroy_branch(b);
	return OK;
}

stat_t populate_kvmem(struct vmem *b)
{
	size_t flags = VM_V | VM_R | VM_W | VM_X | VM_G;
	for (size_t i = KSTART_PAGE; i < IO_PAGE; ++i)
		b->leaf[i] = (struct vmem *)to_pte(
			RAM_BASE + SZ_1G * (i - KSTART_PAGE), flags);

	/* map kernel IO to zero for now, this will be overridden later
	 * (if at all) */
	b->leaf[IO_PAGE] = (struct vmem *)to_pte(0, flags);
	return OK;
}

#if defined(DEBUG)
vm_t setup_kernel_io(struct vmem *b, vm_t paddr)
{
	/* assume Sv39 for now */
	pm_t gigapage = paddr / MM_GPAGE_SIZE;
	b->leaf[IO_PAGE] = (struct vmem *)to_pte(gigapage, VM_V | VM_R | VM_W);
	return -SZ_1G + paddr - (gigapage * MM_GPAGE_SIZE);
}
#endif

stat_t clone_uvmem(struct vmem *r, struct vmem *b)
{
	/** \todo error checking? */
	for (size_t i = 0; i <= CSTACK_PAGE; ++i)
		b->leaf[i] = r->leaf[i];

	return OK;
}
