/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_MEM_H
#define KMI_MEM_H

/**
 * @file mem.h
 * Generic memory, common to both physical and virtual memory.
 */

#include <kmi/utils.h>
#include <kmi/types.h>
#include <arch/mem.h>

/** Maximum number of page orders allowed. Likely massively overkill. */
#define NUM_ORDERS 10

/** Give names to page orders. */
enum mm_order {
	/** NULL marker. */
	MM_MIN = -1,

	/** Base order. */
	MM_O0 = 0,

	/** Order 1. */
	MM_O1 = 1,

	/** Order 2. */
	MM_O2 = 2,

	/** Order 3. */
	MM_O3 = 3,

	/** Order 4. */
	MM_O4 = 4,

	/** Order 5. */
	MM_O5 = 5,

	/** Order 6. */
	MM_O6 = 6,

	/** Order 7. */
	MM_O7 = 7,

	/** Order 8. */
	MM_O8 = 8,

	/** Order 9. */
	MM_O9 = 9,

	/** Number of orders */
	MM_NUM,
};


/**
 * Convert physical memory address \c paddr to index of page order \c order.
 *
 * @param p Physical memory address.
 * @param order Order page index to convert to.
 * @return Index of page order \c order.
 */
#define pm_to_index(p, order) \
	((p >> order_shift(order)) & (order_width(order) - 1))

/**
 * Get highest possible index in an order.
 *
 * @param order Order to query.
 * @return Highest possible index in the order \c order.
 */
#define max_index(order) (order_width(order) - 1)

/**
 * Get starting offset of order bits in address.
 *
 * @param order Order to query.
 * @return Starting offset of order bits.
 */
size_t order_shift(enum mm_order order);

/**
 * Get number of order bits in an address.
 *
 * @param order Order to query.
 * @return Width in bits of order bits.
 */
size_t order_width(enum mm_order order);

/**
 * Get size of order, as in how many pages of one order lower it can contain.
 *
 * @param order Order to query.
 * @return Number of pages of one order lower this order can contain.
 */
size_t order_size(enum mm_order order);

/**
 * Get highest order supported by the current configuration.
 *
 * @return Max supported order.
 */
enum mm_order max_order();

/**
 * Get base page shift.
 *
 * @return Page shift.
 */
size_t page_shift();

/**
 * Get number of elements needed to represent this order.
 *
 * @param order Order to query.
 * @return Number of elements needed to represent this order.
 */
#define order_elems(order) (__mm_widths[order] / MM_OINFO_WIDTH)

/**
 * Entry index to element index that contains the entry.
 *
 * @param idx Index of entry.
 * @return Index of element.
 */
#define order_container(idx) ((idx) / MM_OINFO_WIDTH)

/** \todo Get rid of slightly ugly __* syntax, as these aren't static. */

/**
 * Convert physical address to virtual address in direct mapping.
 *
 * @param x Physical address.
 * @return Corresponding virtual address.
 */
#define __va(x) (void *)(((uintptr_t)(x)) + VM_DMAP)

/**
 * Convert virtual address to physical address in direct mapping.
 *
 * @param x Virtual address.
 * @return Corresponding physical address.
 */
#define __pa(x) (void *)(((uintptr_t)(x)) - VM_DMAP)

/**
 * Get page number of physical address.
 *
 * @param x Physical address.
 * @return Corresponding page number.
 */
#define __page(x) ((x) / BASE_PAGE_SIZE)

/**
 * Get physical address of page number.
 *
 * @param x Page number.
 * @return Corresponding physical address.
 */
#define __addr(x) ((x) * BASE_PAGE_SIZE)

/**
 * Convert bytes to number of base pages.
 *
 * @param x Number of bytes.
 * @return Corresponding number of page pages.
 */
#define __pages(x)                                       \
	(is_aligned((x), BASE_PAGE_SIZE) ? __page((x)) : \
	 __page((x) + BASE_PAGE_SIZE))

/** @name Memory region flags. */
/** @{ */

/** Memory region is used. */
#define MR_USED (1 << (ARCH_VP_FLAGS + 0))
/** Don't free memory on clear. */
#define MR_KEEP (1 << (ARCH_VP_FLAGS + 1))
/** Memory region in shared, but owned. Note: regions that are shared but no
 * owned don't use this flag, they just set the tid field for the region. */
#define MR_SHARED (1 << (ARCH_VP_FLAGS + 2))

/** @} */

/**
 * Find nearest order to size.
 *
 * @param size Size to map to an order.
 * @return Nearest order larger than \p size.
 */
enum mm_order nearest_order(size_t size);

/**
 * Initialize memory subsystem data.
 *
 * @param fdt Pointer to flattened device tree.
 */
void init_mem(void *fdt);

/**
 * Set RAM base address for global access.
 * For now supports only one RAM bank.
 *
 * @param base RAM base address.
 */
void set_ram_base(pm_t base);

/** @param size Set RAM size. */
void set_ram_size(size_t size);

/** @param addr Set load address. */
void set_load_addr(pm_t addr);

/**
 * Get RAM base address.
 * Very much assumes set_ram_base() has been called beforehand.
 *
 * @return RAM base address.
 */
pm_t get_ram_base();

/**
 * Get RAM size.
 * Assumes \ref set_ram_size() has been called beforehand.
 * Does not take into account disjoint RAM regions.
 *
 * @return RAM size.
 */
size_t get_ram_size();

/** @return Load address. */
pm_t get_load_addr();

/** Base page size. Can be overridden by arch if we know it to be some constant
 * value. */
#ifndef BASE_PAGE_SIZE
#define BASE_PAGE_SIZE (order_size(BASE_PAGE))
#endif

/** Base page order. */
#define BASE_PAGE (MM_O0)

#endif /* KMI_MEM_H */
