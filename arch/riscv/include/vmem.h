#ifndef APOS_RISCV_VMAP_H
#define APOS_RISCV_VMAP_H

#include <apos/sizes.h>
#include <apos/types.h>
#include <apos/attrs.h>

#define VM_V (1 << 0)
#define VM_R (1 << 1)
#define VM_W (1 << 2)
#define VM_X (1 << 3)
#define VM_U (1 << 4)
#define VM_G (1 << 5)
#define VM_A (1 << 6)
#define VM_D (1 << 7)

typedef size_t vaddr_t;
struct __packed vm_branch_t {
	struct vm_branch_t *leaf[512];
};

void map_vmem(struct vm_branch_t *branch,
		paddr_t paddr, vaddr_t vaddr,
		uint8_t flags, enum mm_order_t order);

void unmap_vmem(struct vm_branch_t *branch, vaddr_t vaddr, enum mm_order_t order);


#endif /* APOS_RISCV_VMAP_H */
