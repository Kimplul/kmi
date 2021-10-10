#include <apos/string.h>
#include <apos/pmem.h>
#include <pages.h>
#include <vmem.h>

#define pte_ppn(pte) (((paddr_t)(pte)) >> 10)
#define pte_flags(pte) (((paddr_t)(pte)) & 0xff)
#define to_pte(p, f) ((paddr_to_pnum(p) << 10) + (f))
#define pte_addr(pte) (pnum_to_paddr(pte_ppn(pte)))

/* probably not actually this simple, right? */
void map_vmem(struct vm_branch_t *branch,
		paddr_t paddr, vaddr_t vaddr,
		uint8_t flags, enum mm_order_t order)
{
	enum mm_order_t top = MAX_ORDER;
	while(top != order){
		size_t idx = paddr_to_index(vaddr, top);

		if(!branch->leaf[idx]){
			paddr_t new_leaf = alloc_page(MM_KPAGE, 0);
			branch->leaf[idx] = (struct vm_branch_t *)to_pte(new_leaf, VM_V);
			memset((void *)new_leaf, 0, sizeof(struct vm_branch_t));
		}

		branch = (struct vm_branch_t *)pte_addr(branch->leaf[idx]);
		top--;
	}

	size_t idx = paddr_to_index(vaddr, top);
	branch->leaf[idx] = (struct vm_branch_t *)to_pte(paddr, flags);
}

void unmap_vmem(struct vm_branch_t *branch,
		vaddr_t vaddr, enum mm_order_t order)
{
	while(order){
		size_t idx = paddr_to_index(vaddr, order);
		branch = (struct vm_branch_t *)pte_addr(branch->leaf[idx]);
	}

	size_t idx = paddr_to_index(vaddr, order);
	if(branch->leaf[idx])
		free_page(order, pte_addr(branch->leaf[idx]));

	branch->leaf[idx] = 0;
}
