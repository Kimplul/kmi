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



/* general overview of the different functions:
 * (un)map_vmem: map one known page of physical memory to one known page of
 * virtual memory
 *
 * (un)map_vregion: map known physical region to unknown virtual region within
 * start and end
 *
 * (un)map_vsize: map unknown physical region to unknown virtual region
 */

/* defined by arch */
int sp_mem_init(struct sp_reg_root *r, size_t nums);

vm_t alloc_uvmem(struct tcb *r, size_t s, uint8_t flags);
void free_uvmem(struct tcb *r, vm_t a);

vm_t map_fill_region(struct vm_branch_t *b, vm_t start, size_t bytes, uint8_t flags);

size_t uvmem_size();
void set_uvmem_size(size_t s);

#endif /* APOS_VMEM_H */
