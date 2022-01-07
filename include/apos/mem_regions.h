#ifndef APOS_MEM_REGIONS_H
#define APOS_MEM_REGIONS_H

#include <apos/mem.h>
#include <apos/types.h>
#include <apos/sp_tree.h>
#include <arch/vmem.h>

#define mem_container(ptr)\
	container_of(ptr, struct sp_mem, sp_n)

struct sp_reg_root {
	struct sp_root free_regions;
	struct sp_root used_regions;
};

struct sp_mem {
	struct sp_node sp_n;

	struct sp_mem *next;
	struct sp_mem *prev;

	char flags;

	vm_t end;
	vm_t start;
};

stat_t sp_mem_init(struct sp_reg_root *r, vm_t start, size_t arena_size);
vm_t alloc_region(struct sp_reg_root *r, size_t size, size_t *actual_size);
vm_t alloc_fixed_region(struct sp_reg_root *r, vm_t start, size_t size, size_t *actual_size);
stat_t free_region(struct sp_reg_root *r, vm_t start);

struct sp_mem *sp_used_find(struct sp_reg_root *r, vm_t start);
struct sp_mem *sp_find_used_closest(struct sp_reg_root *r, vm_t start);
struct sp_mem *sp_find_free(struct sp_reg_root *r, size_t size, size_t *align);

#define MEM_REGION_TRY_AGAIN 1
typedef stat_t mem_region_callback_t(struct vm_branch *b,
		pm_t *offset, vm_t vaddr, vmflags_t flags, enum mm_order order);

vm_t map_fill_region(struct vm_branch *b, mem_region_callback_t *mem_handler,
		pm_t offset, vm_t start, size_t bytes, vmflags_t flags);

#endif /* APOS_MEM_REGIONS_H */
