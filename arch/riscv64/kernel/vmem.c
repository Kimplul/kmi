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
 * Gravestone marker.
 *
 * Riscv allows us to have arbitrary data in page entries, as long as they're
 * not marked active (VM_V) the content is ignored. Here we use this to our
 * advantage by differentiating between empty entries (NULL) and filler entries
 * (GRAVESTONE).
 *
 * A gravestone tells us that somewhere above it (= higher index) there is an
 * active entry. This is useful mainly in \ref clone_uvmem(), where we can stop
 * copying data as soon as we hit an empty entry. I expect typical programs to
 * generally have most active entries in relatively low addresses, and allowing
 * us to skip copying 'obvious' entries is way quicker than copying the whole
 * 2048 byte user virtual memory.
 *
 * Current optimisations also include setting the uvmem to stop on an 8-page
 * boundary, allowing \ref clone_uvmem() to work in eight page increments for a
 * bit of extra speed. Gravestones are only applied to userspace virtual memory,
 * that is kernel and rpc memory regions are ignored.
 *
 * Example of how stuff should look like:
 *
 * Startin with page entries:
 * 1 2 3 4 0 0 0 ...
 *
 * Mapping a page:
 * 1 2 3 4 0 5 0 ...
 *
 * Adding gravestones:
 * 1 2 3 4 G 5 0 ...
 *
 * More testing is probably necessary, as the init tests program doesn't really
 * excercise the mapping utilities.
 */
#define GRAVESTONE VM_G

/**
 * Check if pte is unused, i.e. either a gravestone or empty.
 *
 * @param b pte to check.
 * @return \ref true if \p b is unused.
 */
static bool __unused(pm_t b)
{
	return b == GRAVESTONE || b == NULL;
}

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

		if (__unused(pte))
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

stat_t set_vpage_flags(struct vmem *branch, vm_t vaddr, vmflags_t flags)
{
	enum mm_order order;
	pm_t *pte = __find_vmem(branch, vaddr, &order);
	if (pte) {
		set_bits(*pte, vp_flags(flags));

		if (order == __mm_max_order)
			return INFO_SEFF;
		else
			return OK;
	}

	return ERR_NF;
}

stat_t clear_vpage_flags(struct vmem *branch, vm_t vaddr, vmflags_t flags)
{
	enum mm_order order;
	pm_t *pte = __find_vmem(branch, vaddr, &order);
	if (pte) {
		clear_bits(*pte, vp_flags(flags));

		if (order == __mm_max_order)
			return INFO_SEFF;
		else
			return OK;
	}

	return ERR_NF;
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
	pm_t new_leaf = alloc_page(MM_KPAGE);
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

/**
 * Add graves if necessary.
 *
 * Checks that the index is within user virtual memory. If it is, change all
 * NULL-entries to gravestones at lower addresses than \p idx.
 *
 * @param branch Top level branch to add graves to.
 * @param idx Index of new entry just added.
 */
static void __add_graves(struct vmem *branch, size_t idx)
{
	if (idx >= CSTACK_PAGE)
		return;

	for (ssize_t i = idx - 1; i >= 0; --i) {
		if (!__unused((pm_t)branch->leaf[i]))
			return;

		branch->leaf[i] = (struct vmem *)GRAVESTONE;
	}
}

stat_t map_vpage(struct vmem *branch, pm_t paddr, vm_t vaddr, vmflags_t flags,
                 enum mm_order order)
{
	struct vmem *root = branch;
	enum mm_order top = __mm_max_order;

	while (top != order) {
		size_t idx = vm_to_index(vaddr, top);

		if (__unused((pm_t)branch->leaf[idx]))
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

	__add_graves(root, vm_to_index(vaddr, __mm_max_order));
	return top == __mm_max_order ? INFO_SEFF : OK;
}

/**
 * Remove graves if possible.
 *
 * Checks if \p idx is in user virtual memory. If it is, check if the entry at
 * \p idx was the top page and was turned into a gravestone. If it was, start
 * removing gravestoned until we hit the next top.
 *
 * @param branch Top level branch to remove gravestones in.
 * @param idx Index of just unmapped page at the top level.
 */
static void __remove_graves(struct vmem *branch, size_t idx)
{
	if (idx >= CSTACK_PAGE)
		return;

	if ((pm_t)branch->leaf[idx + 1] != NULL)
		return;

	for (ssize_t i = idx; i >= 0; --i) {
		if ((pm_t)branch->leaf[i] != GRAVESTONE)
			return;

		branch->leaf[i] = NULL;
	}
}

stat_t unmap_vpage(struct vmem *branch, vm_t vaddr)
{
	pm_t *pte = __find_vmem(branch, vaddr, 0);
	if (pte) {
		*pte = GRAVESTONE;
		__remove_graves(branch, vm_to_index(vaddr, __mm_max_order));
		return OK;
	}

	return ERR_NF;
}

void flush_tlb()
{
	__asm__ volatile ("sfence.vma %0\n" : : "r" (0) : "memory");
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
	pm_t pn = (pm_t)(branch) >> page_shift();

	pm_t mode = DEFAULT_Sv_MODE;

	if (m == Sv32)
		mode = SATP_MODE_Sv32;
	else if (m == Sv39)
		mode = SATP_MODE_Sv39;
	else if (m == Sv48)
		mode = SATP_MODE_Sv48;

	csr_write(CSR_SATP, mode | pn);
	flush_tlb();
	/* Sv57 && Sv64 in the future? */
	/** @todo ASID table for maybe faster context switches? */
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
	struct vmem *b = (struct vmem *)alloc_page(MM_KPAGE);
	memset(b, 0, MM_KPAGE_SIZE);

	populate_kvmem(b);
	return b;
}

stat_t use_vmem(struct vmem *b)
{
	__use_vmem(b, DEFAULT_Sv_MODE);
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
			RAM_BASE + TOP_PAGE_SIZE * (i - KSTART_PAGE), flags);

	/* map in IO region */
	map_io_dbg(b);
	return OK;
}

#if defined(DEBUG)
vm_t setup_kernel_io(struct vmem *b, vm_t paddr)
{
	pm_t top_page = paddr / TOP_PAGE_SIZE;
	b->leaf[IO_PAGE] = (struct vmem *)to_pte(top_page * TOP_PAGE_SIZE,
	                                         VM_V | VM_R | VM_W);
	return -TOP_PAGE_SIZE + paddr - (top_page * TOP_PAGE_SIZE);
}
#endif

void clone_uvmem(struct vmem *r, struct vmem *b)
{
	size_t i = 0;
	for (; i < CSTACK_PAGE; i += 8) {
		struct vmem *t = r->leaf[i + 0];
		if (t == 0)
			break;

		b->leaf[i + 0] = t;
		b->leaf[i + 1] = r->leaf[i + 1];
		b->leaf[i + 2] = r->leaf[i + 2];
		b->leaf[i + 3] = r->leaf[i + 3];
		b->leaf[i + 4] = r->leaf[i + 4];
		b->leaf[i + 5] = r->leaf[i + 5];
		b->leaf[i + 6] = r->leaf[i + 6];
		b->leaf[i + 7] = r->leaf[i + 7];
	}

	for (; i < CSTACK_PAGE; i += 8) {
		struct vmem *t = b->leaf[i + 0];
		if (t == 0)
			break;

		b->leaf[i + 0] = 0;
		b->leaf[i + 1] = 0;
		b->leaf[i + 2] = 0;
		b->leaf[i + 3] = 0;
		b->leaf[i + 4] = 0;
		b->leaf[i + 5] = 0;
		b->leaf[i + 6] = 0;
		b->leaf[i + 7] = 0;
	}
}
