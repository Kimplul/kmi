/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_REGIONS_H
#define KMI_REGIONS_H

/**
 * @file regions.h
 * Memory region subsytem. Mainly used by the virtual memory subsytems, i.e. device and
 * user memory.
 */

#include <kmi/mem.h>
#include <kmi/types.h>
#include <kmi/nodes.h>
#include <kmi/sp_tree.h>

#include <arch/vmem.h>

/**
 * Initialize memory region nodes. Must be done after physical memory has been
 * initialized.
 */
void init_mem_nodes();

/**
 * Destroy all nodes allocated by the memory region subsystem.
 */
void destroy_mem_nodes();

/**
 * Get \ref mem_region container of \c ptr.
 *
 * @param ptr Pointer to \c sp_n member in \ref mem_region.
 * @return Parent \ref mem_region.
 */
#define mem_container(ptr) container_of(ptr, struct mem_region, sp_n)

/**
 * Check if memory region is used.
 *
 * @param r Memory region to check.
 * @return \c 0 if not used, non-zero otherwise.
 */
#define is_region_used(r) is_set(r->flags, MR_USED)

/**
 * Check if region should be kept during clear.
 *
 * @param r Memory region to check.
 * @return \c 0 if not kept, non-zero otherwise.
 */
#define is_region_kept(r) is_set(r->flags, MR_KEEP)

/** Root of memory region. */
struct mem_region_root {
	/** How many pages are reserved, i.e. will never be allocated by \ref
	 * alloc_region(), they have to explicitly be requested for by
	 * \ref alloc_fixed_region(). */
	size_t reserved;

	/** Helper for reserved calculations. */
	vm_t start;

	/** Currently unused. */
	vm_t end;

	/** Sp-tree of free regions. */
	struct sp_root free_regions;

	/** Sp-tree of used region. */
	struct sp_root used_regions;
};

/**
 * Memory region.
 * Regions can have two states, used or free. There are two sp-trees, which keep
 * track of free and used regions, respectively. All regions are chained
 * together with a doubly linked list, so that the next region's start address
 * should be the current region's end address.
 *
 * Note that addresses are expressed in pages!
 */
struct mem_region {
	/** Sp-tree node slot. */
	struct sp_node sp_n;

	/** Next memory region by start address. */
	struct mem_region *next;

	/** Previous memory region by end address. */
	struct mem_region *prev;

	/** End address of memory region. */
	vm_t end;

	/** Start address of memory region. */
	vm_t start;

	/** In shared regions, mark the other pid that shared the region. */
	id_t pid;

	union {
		vm_t shaddr;
		size_t refcount;
	};

	/** Memory region flags, both access as well as metadata. \see MR_USED,
	 * MR_SHARED, MR_OWNED, MR_COW, MR_KEEP. */
	vmflags_t flags;

};

/**
 * Initialize memory region subsystem instance.
 *
 * @param r Memory region root to initialize.
 * @param start Start of memory arena.
 * @param arena_size Size of memory arena.
 * @param reserved Size of reserved area. The reserved area will never be
 * allocated from with \ref alloc_region(), the user has to explicitly ask to
 * use that region.
 * @return \ref OK on success.
 * \todo Document error codes when I actually implement them properly.
 */
stat_t init_region(struct mem_region_root *r, vm_t start, size_t arena_size,
                   size_t reserved);

/**
 * Destroy memory region subsystem instance.
 *
 * @param r Memory region root to destroy.
 * @return \ref OK on success.
 */
stat_t destroy_region(struct mem_region_root *r);

/**
 * Allocate memory region.
 * Will allocate region of at least \c size bytes, with best possible location.
 *
 * @param r Memory region root.
 * @param size Size of region to allocate.
 * @param actual_size Size of region that was allocated.
 * @param flags Memory flags.
 * @return Address of allocated region on success, otherwise \c NULL.
 */
vm_t alloc_region(struct mem_region_root *r, size_t size, size_t *actual_size,
                  vmflags_t flags);

/**
 * Allocate memory region and associate it with some other process.
 * Will allocate region of at least \c size bytes, with best possible location.
 *
 * @param r Memory region root.
 * @param size Size of region to allocate.
 * @param actual_size Size of region that was allocated.
 * @param flags Memory flags.
 * @param pid Process to associate with region.
 * @return Address of allocated region on success, otherwise \c NULL.
 */
vm_t alloc_shared_region(struct mem_region_root *r, size_t size,
                         size_t *actual_size,
                         vmflags_t flags, id_t pid);

/**
 * Allocate a shred memory region at a fixed virtual address and associate it with
 * some other process.
 *
 * Will allocate region of at least \c size bytes, with best possible location.
 *
 * @param r Memory region root.
 * @param start Start of region to allocate.
 * @param size Size of region to allocate.
 * @param actual_size Size of region that was allocated.
 * @param flags Memory flags.
 * @param pid Process to associate with region.
 * @return Address of allocated region on success, otherwise \c NULL.
 */
vm_t alloc_shared_fixed_region(struct mem_region_root *r, vm_t start,
                               size_t size,
                               size_t *actual_size, vmflags_t flags, id_t pid);

/**
 * Allocate fixed memory region.
 * Will allocate region that is at least \c size bytes, and includes \c start.
 * \note The address returned might not be the address requested, and the caller
 * must keep track of which address it was given, so it can cleanly give it to
 * \ref free_region() when finished with the allocation.
 *
 * @param r Memory region root.
 * @param start Address that must be within region to allocate.
 * @param size Size of region to allocate.
 * @param actual_size Size of region that was allocated.
 * @param flags Memory flags.
 * @return Address of allocated region on success, otherwise \c NULL.
 */
vm_t alloc_fixed_region(struct mem_region_root *r, vm_t start, size_t size,
                        size_t *actual_size, vmflags_t flags);

/**
 * Free memory region.
 *
 * @param r Memory region root.
 * @param start Address of region to free.
 * @return \ref OK on success, \ref ERR_ALIGN if \c start is misaligned and \ref
 * ERR_NF if the memory region is not found.
 */
stat_t free_region(struct mem_region_root *r, vm_t start);

/**
 * Free memory region through a direct pointer to the memory region.
 * Mainly useful if you look up a region beforehand, do something with it and
 * then free it. Skips looking up the start address.
 *
 * @param r Memory region root.
 * @param m Memory region to free.
 */
void free_known_region(struct mem_region_root *r, struct mem_region *m);

/**
 * Find the memory region with lowest starting address.
 * This region will also be the first node in the linked list.
 * Useful when you need to iterate over all regions.
 *
 * @param r Memory region root.
 * @return First memory region when succesful, otherwise \c NULL.
 */
struct mem_region *find_first_region(struct mem_region_root *r);

/**
 * Find used memory region at address \c start.
 *
 * @param r Memory region root.
 * @param start Address at which a used region should exist.
 * @return Pointer to requested memory region when succesful, \c NULL otherwise.
 */
struct mem_region *find_used_region(struct mem_region_root *r, vm_t start);

/**
 * Find used memory region closest to \c start.
 * Useful when you don't necessarily need the exact region, just something close
 * by.
 *
 * @param r Memory region root.
 * @param start Address region should be closest to.
 * @return Pointer to memory region closest to \c start when succesful, \c NULL
 * otherwise.
 */
struct mem_region *find_closest_used_region(struct mem_region_root *r,
                                            vm_t start);

/**
 * Find best region that fulfills requested parameters.
 * Respects the reserved region of \p r.
 *
 * @param r Memory region root.
 * @param size Size of free region.
 * @param align Recommended offset into this region from where the allocation
 * should be carved.
 * @return Pointer to suitable \c memory_region when succesful, \c NULL
 * otherwise.
 */
struct mem_region *find_free_region(struct mem_region_root *r, size_t size,
                                    size_t *align);

/**
 * Map a region starting at virtual address \p start, that is \p bytes bytes in size
 * to physical memory. Will allocate pages itself. If it fails midway through,
 * doesn't clean up after itself, so remember to call \ref unmap_region() in
 * such a case.
 *
 * @param vmem Virtual memory to do allocation in.
 * @param start Start of region.
 * @param bytes How large the allocation is in bytes.
 * @param order What is the maximum order of page the function is allowed to
 * use. This is mainly useful for shared memory regions that may want to always
 * use base pages to maximize the chance of a clone succeeding.
 * @param flags What flags to assign the page.
 * @return \ref OK on success, some other error code otherwise.
 */
stat_t map_region(struct vmem *vmem, vm_t start, size_t bytes,
                  enum mm_order order, vmflags_t flags);

/**
 * Map a virtual memory region starting at \p v to the physical memory region
 * starting at \p start, \p bytes long. Same as \ref map_region(), clean up
 * after this function if it fails.
 *
 * @param vmem Virtual memory to do mapping in.
 * @param v Start of virtual region.
 * @param start Start of physical region.
 * @param bytes Size of region in bytes.
 * @param flags Flags to use for mapping.
 * @return \ref OK on success, some other error code otherwise.
 */
stat_t map_fixed_region(struct vmem *vmem, vm_t v, pm_t start, size_t bytes,
                        vmflags_t flags);

/**
 * Clone a region starting at \p from in \p g of size \p bytes into \p to in \p
 * b. Does not allocate new pages, just makes the virtual region point to the
 * same physical pages. Used to implement shared memory, primarily.
 *
 * @param b Where to create mapping.
 * @param g Where current mapping exists.
 * @param from Start of region in \p g.
 * @param to Start of region in \p b.
 * @param bytes Size of region. Must be identical for both \p b and \p g.
 * @param flags Flags for the new mapping. The original mapping can be RW while
 * the new one just R, for example.
 *
 * @return \ref OK on success, some other error code otherwise.
 */
stat_t clone_region(struct vmem *b, struct vmem *g, vm_t from, vm_t to,
                    size_t bytes, vmflags_t flags);

/**
 * Copy region starting at \p from in \p g of size \p bytes to \p to in \p b.
 * Allocates new pages, so the regions get copied as well. Used to implement
 * \ref fork().
 *
 * @param b Where to create mapping.
 * @param g Where current mapping exists.
 * @param from Start of region in \p g.
 * @param to Start of region in \p b.
 * @param bytes Size of region. Must be identical for both \p b and \p g.
 *
 * @return \ref OK on success, some other error code otherwise.
 */
stat_t copy_region(struct vmem *b, struct vmem *g, vm_t from, vm_t to,
                   size_t bytes);

/**
 * Unmap a region, while at the same time freeing backing pages.
 *
 * @param b Where to unmap region.
 * @param v Start of region to unmap.
 * @param bytes Size of region to unmap.
 */
void unmap_region(struct vmem *b, vm_t v, size_t bytes);

/**
 * Unmap a region, without freeing any pages.
 * Useful for freeing shared memory or mappings outside RAM.
 *
 * @param b Where to unmap region.
 * @param v Start of region to unmap.
 * @param bytes Size of region to unmap.
 */
void unmap_fixed_region(struct vmem *b, vm_t v, size_t bytes);

#endif /* KMI_REGIONS_H */
