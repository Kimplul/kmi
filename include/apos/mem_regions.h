#ifndef APOS_MEM_REGIONS_H
#define APOS_MEM_REGIONS_H

#include <apos/mem.h>
#include <apos/types.h>
#include <apos/sp_tree.h>
#include <arch/vmem.h>

#define mem_container(ptr)\
	container_of(ptr, struct mem_region, sp_n)

struct mem_region_root {
	struct sp_root free_regions;
	struct sp_root used_regions;
};

struct mem_region {
	struct sp_node sp_n;

	struct mem_region *next;
	struct mem_region *prev;

	char flags;

	vm_t end;
	vm_t start;
};

stat_t init_region(struct mem_region_root *r, vm_t start, size_t arena_size);
void destroy_region(struct mem_region_root *r);

vm_t alloc_region(struct mem_region_root *r, size_t size, size_t *actual_size);
vm_t alloc_fixed_region(struct mem_region_root *r, vm_t start, size_t size, size_t *actual_size);
stat_t free_region(struct mem_region_root *r, vm_t start);

struct mem_region *find_used_region(struct mem_region_root *r, vm_t start);
struct mem_region *find_closest_used_region(struct mem_region_root *r, vm_t start);
struct mem_region *find_free_region(struct mem_region_root *r, size_t size, size_t *align);

#define REGION_TRY_AGAIN 1
typedef stat_t region_callback_t(struct vm_branch *b,
		pm_t *offset, vm_t vaddr, vmflags_t flags, enum mm_order order);

vm_t map_fill_region(struct vm_branch *b, region_callback_t *mem_handler,
		pm_t offset, vm_t start, size_t bytes, vmflags_t flags);

#endif /* APOS_MEM_REGIONS_H */
