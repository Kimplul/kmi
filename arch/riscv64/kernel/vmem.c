#include <apos/string.h>
#include <apos/pmem.h>
#include <apos/vmem.h>
#include <apos/mem.h>
#include <apos/debug.h>
#include <arch/cpu.h>
#include <pages.h>
#include <csr.h>

#define pte_ppn(pte)      (((pm_t)(pte)) >> 10)
#define pte_flags(pte)    (((pm_t)(pte)) & 0xff)
#define to_pte(p, f)      ((pm_to_pnum(p) << 10) | (f))
#define pte_addr(pte)     __va(pnum_to_paddr(pte_ppn(pte)))
#define pte_paddr(pte)    (pnum_to_paddr(pte_ppn(pte)))
#define vm_to_index(a, o) (pm_to_index(a, o))
#define is_active(pte)    (pte_flags(pte) & VM_V)
#define is_leaf(pte)      (is_active(pte) && (pte_flags(pte) & ~VM_V))
#define is_branch(pte)    (is_active(pte) && !(pte_flags(pte) & ~VM_V))

static pm_t *__find_vmem(struct vm_branch *b, vm_t v, enum mm_order *o)
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

		b = (struct vm_branch *)pte_addr(pte);
	} while (top--);

	return 0;
}

stat_t mod_vpage(struct vm_branch *branch, vm_t vaddr, pm_t paddr,
                 vmflags_t flags)
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
stat_t stat_vpage(struct vm_branch *branch, vm_t vaddr, pm_t *paddr,
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

static struct vm_branch *__create_leaf()
{
	pm_t new_leaf = alloc_page(MM_KPAGE, 0);
	memset((void *)new_leaf, 0, sizeof(struct vm_branch));
	return (struct vm_branch *)to_pte((pm_t)__pa(new_leaf), VM_V);
}

static void __destroy_branch(struct vm_branch *b)
{
	if (!b)
		return;

	for (size_t i = 0; i < BASE_PAGE_SIZE / sizeof(pm_t); ++i) {
		if (is_branch(b->leaf[i]))
			__destroy_branch(
				(struct vm_branch *)pte_addr(b->leaf[i]));

		free_page(MM_KPAGE, (pm_t)pte_addr(b->leaf[i]));
	}
}

stat_t map_vpage(struct vm_branch *branch, pm_t paddr, vm_t vaddr,
                 vmflags_t flags, enum mm_order order)
{
	enum mm_order top = __mm_max_order;
	while (top != order) {
		size_t idx = vm_to_index(vaddr, top);

		if (!branch->leaf[idx])
			branch->leaf[idx] = __create_leaf();

		branch = (struct vm_branch *)pte_addr(branch->leaf[idx]);
		top--;
	}

	size_t idx = vm_to_index(vaddr, top);
	if (is_branch(
		    branch->leaf[idx])) /* something has gone terribly wrong? */
		__destroy_branch(branch->leaf[idx]);

	branch->leaf[idx] =
		(struct vm_branch *)to_pte((pm_t)__pa(paddr), vp_flags(flags));

	return top == __mm_max_order ? INFO_SEFF : OK;
}

stat_t unmap_vpage(struct vm_branch *branch, vm_t vaddr)
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
	__asm__ volatile("sfence.vma %0\n" ::"r"(cpu_id()) : "memory");
}

void flush_tlb_all()
{
	__asm__ volatile("sfence.vma\n" ::: "memory");
}

static void __start_vmem(struct vm_branch *branch, enum mm_mode m)
{
	branch = (struct vm_branch *)__pa(branch);

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

struct vm_branch *init_vmem(void *fdt)
{
	UNUSED(fdt);
	struct vm_branch *b = (struct vm_branch *)alloc_page(MM_KPAGE, 0);
	memset(b, 0, MM_KPAGE_SIZE);

	populate_root_branch(b);
	/* update which memory branch to use */
	__start_vmem(b, Sv39);
	return b;
}

void populate_root_branch(struct vm_branch *b)
{
	size_t flags = VM_V | VM_R | VM_W | VM_X | VM_G;
	for (size_t i = KSTART_PAGE; i < IO_PAGE; ++i)
		b->leaf[i] = (struct vm_branch *)to_pte(
			RAM_BASE + SZ_1G * (i - KSTART_PAGE), flags);

	/* map kernel IO to zero for now, this will be overridden later
	 * (if at all) */
	b->leaf[IO_PAGE] = (struct vm_branch *)to_pte(0, flags);
}

#if defined(DEBUG)
vm_t setup_kernel_io(struct vm_branch *b, vm_t paddr)
{
	/* assume Sv39 for now */
	pm_t gigapage = paddr / MM_GPAGE_SIZE;
	b->leaf[IO_PAGE] =
		(struct vm_branch *)to_pte(gigapage, VM_V | VM_R | VM_W);
	return -SZ_1G + paddr - (gigapage * MM_GPAGE_SIZE);
}
#endif

stat_t clone_vmbranch(struct vm_branch *r, struct vm_branch *b)
{
	/* TODO: error checking? */
	for (size_t i = 0; i <= CSTACK_PAGE; ++i)
		b->leaf[i] = r->leaf[i];

	return OK;
}
