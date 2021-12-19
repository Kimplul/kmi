#include <apos/string.h>
#include <apos/pmem.h>
#include <apos/vmem.h>
#include <apos/mem.h>
#include <apos/debug.h>
#include <pages.h>

#define pte_ppn(pte) (((pm_t)(pte)) >> 10)
#define pte_flags(pte) (((pm_t)(pte)) & 0xff)
#define to_pte(p, f) ((pm_to_pnum(p) << 10) + (f))
#define pte_addr(pte) (pnum_to_paddr(pte_ppn(pte)))
#define vm_to_index(a, o) (pm_to_index(a, o))

void map_vmem(struct vm_branch_t *branch,
		pm_t paddr, vm_t vaddr, uint8_t flags, enum mm_order_t order)
{
	enum mm_order_t top = __mm_max_order;
	while (top != order) {
		size_t idx = vm_to_index(vaddr, top);

		if (!branch->leaf[idx]) {
			pm_t new_leaf = alloc_page(MM_KPAGE, 0);
			branch->leaf[idx] =
				(struct vm_branch_t *)to_pte(new_leaf, VM_V);

			void *leaf_ptr = (void *)new_leaf;
			memset(leaf_ptr, 0, sizeof(struct vm_branch_t));
		}

		pm_t pte = (pm_t)branch->leaf[idx];
		pm_t branch_pptr = (pm_t)pte_addr(pte);
		branch = (struct vm_branch_t *)branch_pptr;
		top--;
	}

	size_t idx = vm_to_index(vaddr, top);
	branch->leaf[idx] = (struct vm_branch_t *)to_pte(paddr, flags);
}

void unmap_vmem(struct vm_branch_t *branch, vm_t vaddr, enum mm_order_t order)
{
	while (order) {
		size_t idx = pm_to_index(vaddr, order);
		branch = (struct vm_branch_t *)pte_addr(branch->leaf[idx]);
	}

	size_t idx = pm_to_index(vaddr, order);
	if (branch->leaf[idx])
		free_page(order, pte_addr(branch->leaf[idx]));

	branch->leaf[idx] = 0;
}
