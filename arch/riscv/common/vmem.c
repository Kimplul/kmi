#include <apos/string.h>
#include <apos/pmem.h>
#include <apos/vmem.h>
#include <pages.h>

#define pte_ppn(pte) (((pm_t)(pte)) >> 10)
#define pte_flags(pte) (((pm_t)(pte)) & 0xff)
#define to_pte(p, f) ((pm_to_pnum(p) << 10) + (f))
#define pte_addr(pte) (pnum_to_paddr(pte_ppn(pte)))

/* probably not actually this simple, right? */
void map_vmem(struct vm_branch_t *branch,
		pm_t paddr, vm_t vaddr,
		uint8_t flags, enum mm_order_t order)
{
	enum mm_order_t top = __mm_max_order;
	while(top != order){
		size_t idx = pm_to_index(vaddr, top);

		if(!branch->leaf[idx]){
			pm_t new_leaf = alloc_page(MM_KPAGE, 0);
			branch->leaf[idx] = (struct vm_branch_t *)to_pte(new_leaf, VM_V);
			memset((void *)new_leaf, 0, sizeof(struct vm_branch_t));
		}

		branch = (struct vm_branch_t *)pte_addr(branch->leaf[idx]);
		top--;
	}

	size_t idx = pm_to_index(vaddr, top);
	branch->leaf[idx] = (struct vm_branch_t *)to_pte(paddr, flags);
}

void unmap_vmem(struct vm_branch_t *branch,
		vm_t vaddr, enum mm_order_t order)
{
	while(order){
		size_t idx = pm_to_index(vaddr, order);
		branch = (struct vm_branch_t *)pte_addr(branch->leaf[idx]);
	}

	size_t idx = pm_to_index(vaddr, order);
	if(branch->leaf[idx])
		free_page(order, pte_addr(branch->leaf[idx]));

	branch->leaf[idx] = 0;
}

vm_t map_vregion(struct vm_branch_t *branch, pm_t base, pm_t top, vm_t start, vm_t end)
{
	/* TODO: figure out how to find first suitable memory region */

	/* find first free page, iterate forward until we either fit the whole
	 * physical region or hit a used page
	 *
	 * continue until we hit end?
	 */
}

void unmap_vregion(struct vm_branch_t *branch, pm_t base, pm_t top)
{
	/* TODO */
}

vm_t map_vsize(struct vm_branch_t *branch, size_t size)
{
	/* TODO */
}

void unmap_vsize(struct vm_branch_t *branch, size_t size)
{
	/* TODO */
}
