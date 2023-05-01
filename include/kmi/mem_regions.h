/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_MEM_REGIONS_H
#define KMI_MEM_REGIONS_H

/**
 * @file mem_regions.h
 * Memory region subsytem. Mainly used by the virtual memory subsytems, i.e. device and
 * user memory.
 */

#include <kmi/mem.h>
#include <arch/vmem.h>
#include <kmi/types.h>
#include <kmi/sp_tree.h>

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
 */
struct mem_region {
	/** Sp-tree node slot. */
	struct sp_node sp_n;

	/** Next memory region by start address. */
	struct mem_region *next;

	/** Previous memory region by end address. */
	struct mem_region *prev;

	/** Memory region flags, both access as well as metadata. \see MR_USED,
	 * MR_SHARED, MR_OWNED, MR_COW, MR_KEEP. */
	vmflags_t flags;

	/** End address of memory region. */
	vm_t end;

	/** Start address of memory region. */
	vm_t start;

	/** In shared regions, mark the other pid that shared the region. */
	id_t pid;

	/** In shared regions, this is the address associated with region in the
	 * other process. */
	vm_t alt_va;
};

/**
 * Initialize memory region subsystem instance.
 *
 * @param r Memory region root to initialize.
 * @param start Start of memory arena.
 * @param arena_size Size of memory arena.
 * @return \ref OK on success.
 * \todo Document error codes when I actually implement them properly.
 */
stat_t init_region(struct mem_region_root *r, vm_t start, size_t arena_size);

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
 * @return \ref OK.
 * \todo Improve error checking.
 */
stat_t free_known_region(struct mem_region_root *r, struct mem_region *m);

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
 * Helper functions for converting between memory regions and page
 * mappings. Called by \ref map_fill_region().
 * \see map_fill_region() for further explanation.
 *
 * @param vmem Virtual memory space in which the operation is to be done.
 * @param offset Offset from where to start looking for next physical page. \see
 * alloc_page().
 * @param vaddr Current virtual address.
 * @param flags Virtual memory allocation flags.
 * @param order Order of physical page.
 * @param data Custom data.
 * @return \ref OK if succesful, \c INFO_TRGN if page order should be decreased,
 * anything else means error.
 */
typedef stat_t region_callback_t(struct vmem *vmem, pm_t *offset, vm_t vaddr,
                                 vmflags_t flags, enum mm_order order,
                                 void *data);

/**
 * Query flags of region that contains \c va.
 *
 * @param r Memory region root.
 * @param va Address that is within memory region.
 * @param flags Memory region flags.
 * @return \ref OK when succesful.
 * \todo Implement.
 */
stat_t stat_region(struct mem_region_root *r, vm_t va, vmflags_t *flags);

/**
 * Modify flags of region that contains \c va.
 *
 * @param r Memory region root.
 * @param va Address that is within memory region.
 * @param flags New flags of region.
 * @return \ref OK when succesful.
 * \todo Implement.
 */
stat_t mod_region(struct mem_region_root *r, vm_t va, vmflags_t flags);

/**
 * Set alternate virtual address associated with shared memory region in other process.
 *
 * @param r Memory region root.
 * @param va Virtual address in \p r.
 * @param alt_va Virtual address in other process.
 */
void set_alt_region_addr(struct mem_region_root *r, vm_t va, vm_t alt_va);

/**
 * Conversion function between abstract memory region and actual page mappings.
 * Assumes that pages of some order are mappable at multiples of their size, and
 * tries to fit as many and as high order pages as it can into the region.
 * Heavily utilizes \c mem_handler, to which it gives a suggestion for how to
 * map a page. If this suggestion is accepted and succesfully executed, \c mem_handler
 * returns \ref OK. If the suggestion is not possible, for example no higher
 * order pages are available, \c mem_handler returns \ref INFO_TRGN to tell \c
 * map_fill_region() to give it some other suggestion. Any error value stops the
 * conversion.
 *
 * This 'algorithm' also works quite nicely for freeing a region, but in
 * reverse, i.e. it is given a suggestion and checks if that suggestion was
 * executed when the region was mapped. If the suggestion was executed, then the
 * same suggestion is freed, else \ref INFO_TRGN is returned and a new
 * suggestion is requested until all pages have been freed.
 *
 * There are some more technicalities, for example currently \c
 * map_fill_region() gives up trying to map higher order pages as soon as its
 * first suggestion is rejected, which gives us quick conversion times but
 * probably less than ideal mappings.
 *
 * @param vmem Virtual memory inside which to map the region.
 * @param mem_handler Worker handler callback.
 * @param offset Offset at which the physical memory availability map should
 * start searching.
 * @param start Start address of memory region.
 * @param bytes Size of memory region.
 * @param flags Memory flags.
 * @param data User-specified data.
 * @return \c start when succesful, \c 0 otherwise.
 */
vm_t map_fill_region(struct vmem *vmem, region_callback_t *mem_handler,
                     pm_t offset, vm_t start, size_t bytes, vmflags_t flags,
                     void *data);

#endif /* KMI_MEM_REGIONS_H */
