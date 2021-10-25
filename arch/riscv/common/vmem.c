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

#if defined(KERNEL)

static vm_t *tmp_pte = 0;
void arch_init_vmem(struct vm_branch_t *branch, vm_t pte)
{
	(void)branch;
	tmp_pte = (vm_t *)pte;
}

/* this seems like an awful idea, should probably try to come up with a smarter
 * approach? flushing the cache every time is probably pretty slow and also
 * this code doesn't take into account the cpu ASID, although that's probably
 * not relevant here */

/* what I probably should do is something like Linux, where the kernel has a
 * direct (or linear, I suppose?) so that all direct memory accesses are also
 * accessible from virtual memory. Not entirely sure how I should do that,
 * wouldn't there be situations where the RAM is larger than the allocated
 * memory region?
 *
 * Or maybe some kind of walker, where I maintain a virtual page table that
 * mirrors the physical page table? Not sure yet, but this seems to work for
 * _now_.
 */
#define QUOTE2(x) #x
#define QUOTE(x) QUOTE2(x)
static void *set_vptr(uint8_t flags, pm_t a)
{
	*tmp_pte = to_pte(a, flags);
	__asm__ ("sfence.vma %0, %1"
			:: "rk" (0), "rK" (TMP_PTE)
			: "memory");

	return (void *)TMP_PTE;
}

#else
struct vm_branch_t *arch_get_tmp_pte(struct vm_branch_t *branch)
{
	enum mm_order_t top = __mm_max_order;
	while(top){
		size_t idx = vm_to_index(TMP_PTE, top);
		branch = (struct vm_branch_t *)pte_addr(branch->leaf[idx]);
		top--;
	}

	return branch;
}

#define set_vptr(flags, a) (a)
#endif

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

			void *leaf_ptr = (void *)set_vptr(VM_R | VM_W | VM_V, new_leaf);
			memset(leaf_ptr, 0, sizeof(struct vm_branch_t));
		}

		pm_t pte = (pm_t)branch->leaf[idx];
		pm_t branch_pptr = (pm_t)pte_addr(pte);
		branch = (struct vm_branch_t *)set_vptr(VM_R | VM_W | VM_V, branch_pptr);
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
