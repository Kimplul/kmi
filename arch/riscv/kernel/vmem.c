#include <apos/string.h>
#include <apos/pmem.h>
#include <apos/vmem.h>
#include <apos/mem.h>
#include <apos/debug.h>
#include <pages.h>
#include <vmem.h>

#define pte_ppn(pte) (((pm_t)(pte)) >> 10)
#define pte_flags(pte) (((pm_t)(pte)) & 0xff)
#define to_pte(p, f) ((pm_to_pnum((pm_t)__pa(p)) << 10) | (f))
#define pte_addr(pte) __va(pnum_to_paddr(pte_ppn(pte)))
#define pte_paddr(pte) (pnum_to_paddr(pte_ppn(pte)))
#define vm_to_index(a, o) (pm_to_index(a, o))
#define is_active(pte) (pte_flags(pte) & VM_V)
#define is_leaf(pte) (is_active(pte) && (pte_flags(pte) & ~VM_V))
#define is_branch(pte) (is_active(pte) && !(pte_flags(pte) & ~VM_V))

static pm_t *__find_vmem(struct vm_branch_t *b, vm_t v, enum mm_order_t *o)
{
	enum mm_order_t top = __mm_max_order;
	if(o)
		*o = MM_O0;
	do {
		size_t idx = vm_to_index(v, top);
		pm_t pte = (pm_t)b->leaf[idx];

		if(!pte)
			return 0;

		if(is_leaf(pte)){
			if(o)
				*o = top;

			return (pm_t *)&b->leaf[idx];
		}

		b = (struct vm_branch_t *)pte_addr(pte);
	} while (top--);

	return 0;
}

int mod_vmem(struct vm_branch_t *branch, vm_t vaddr, pm_t paddr, uint8_t flags)
{
	pm_t *pte = __find_vmem(branch, vaddr, 0);
	if(pte){
		*pte = to_pte(paddr, flags);
		return 0;
	}

	return -1;
}

/* huh, should probably add status flags etc. to all my API functions. Damn, I'm
 * lazy. */
int stat_vmem(struct vm_branch_t *branch, vm_t vaddr, pm_t *paddr,
		enum mm_order_t *order, uint8_t *flags)
{
	pm_t *pte = __find_vmem(branch, vaddr, order);
	if(pte){
		if(paddr)
			*paddr = (pm_t)pte_addr(*pte);

		if(flags)
			*flags = pte_flags(*pte);

		return 0;
	}

	return -1;
}

static struct vm_branch_t *__create_leaf()
{
	pm_t new_leaf = alloc_page(MM_KPAGE, 0);
	memset((void *)new_leaf, 0, sizeof(struct vm_branch_t));
	return (struct vm_branch_t *)to_pte(new_leaf, VM_V);
}

static void __destroy_branch(struct vm_branch_t *b)
{
	if(!b)
		return;

	for(size_t i = 0; i < BASE_PAGE_SIZE / sizeof(pm_t); ++i){
		if(is_branch(b->leaf[i]))
			__destroy_branch((struct vm_branch_t *)pte_addr(b->leaf[i]));

		free_page(MM_KPAGE, (pm_t)pte_addr(b->leaf[i]));
	}
}

void map_vmem(struct vm_branch_t *branch,
		pm_t paddr, vm_t vaddr, uint8_t flags, enum mm_order_t order)
{
	enum mm_order_t top = __mm_max_order;
	while (top != order) {
		size_t idx = vm_to_index(vaddr, top);

		if (!branch->leaf[idx])
			branch->leaf[idx] = __create_leaf();


		branch = (struct vm_branch_t *)pte_addr(branch->leaf[idx]);
		top--;
	}

	size_t idx = vm_to_index(vaddr, top);
	if (is_branch(branch->leaf[idx])) /* something has gone terribly wrong */
		__destroy_branch(branch->leaf[idx]);

	branch->leaf[idx] = (struct vm_branch_t *)to_pte(paddr, flags);
}

void unmap_vmem(struct vm_branch_t *branch, vm_t vaddr)
{

	pm_t *pte = __find_vmem(branch, vaddr, 0);
	if(pte)
		*pte = 0;
}
