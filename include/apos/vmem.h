#ifndef APOS_VMEM_H
#define APOS_VMEM_H

struct sp_mem;

/* arch-specific data */
#include <vmem.h>

/* common */
#include <apos/pmem.h>
#include <apos/tcb.h>
#include <apos/sp_tree.h>

struct sp_mem {
	struct sp_node sp_n;

	struct sp_mem *next;
	struct sp_mem *prev;

	char flags;

	vm_t end;
	vm_t start;
};

#define mem_container(ptr)\
	container_of(ptr, struct sp_mem, sp_n)

#define __va(x) (((char *)(x)) + VM_DMAP - RAM_BASE)
#define __pa(x) (((char *)(x)) + RAM_BASE - VM_DMAP)
#define __page(x) ((x) / BASE_PAGE_SIZE)
#define __addr(x) ((x) * BASE_PAGE_SIZE)
#define __pages(x) (aligned((x), BASE_PAGE_SIZE) ? __page((x)) : __page((x) + BASE_PAGE_SIZE))
#define __bytes(x) (__addr(x))

/* general overview of the different functions:
 * internally they all work with pages, but they are called and return usable
 * addresses, though within page bounds of course.
 */

/* defined by arch */
void map_vmem(struct vm_branch_t *branch,
		pm_t paddr, vm_t vaddr, uint8_t flags, enum mm_order_t order);

void unmap_vmem(struct vm_branch_t *branch, vm_t vaddr);
int mod_vmem(struct vm_branch_t *branch, vm_t vaddr, pm_t paddr, uint8_t flags);
int stat_vmem(struct vm_branch_t *branch, vm_t vaddr, pm_t *paddr,
		enum mm_order_t *order, uint8_t *flags);

int sp_mem_init(struct sp_reg_root *r, vm_t start, size_t nums);

vm_t alloc_region(struct sp_reg_root *r, size_t size, size_t *actual_size);
vm_t alloc_fixed_region(struct sp_reg_root *r, vm_t start, size_t size, size_t *actual_size);
void free_region(struct sp_reg_root *r, vm_t start);

vm_t alloc_uvmem(struct tcb *r, size_t size, uint8_t flags);
vm_t alloc_fixed_uvmem(struct tcb *r, vm_t start, size_t size, uint8_t flags);
void free_uvmem(struct tcb *r, vm_t a);

vm_t map_fill_region(struct vm_branch_t *b, vm_t start, size_t bytes, uint8_t flags);

size_t uvmem_size();
void set_uvmem_size(size_t s);

#endif /* APOS_VMEM_H */
