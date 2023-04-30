/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_MEM_H
#define KMI_MEM_H

/**
 * @file mem.h
 * Generic memory, common to both physical and virtual memory.
 */

#include <kmi/utils.h>
#include <kmi/types.h>

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
#define order_shift(order) (__mm_shifts[order])

/**
 * Get number of order bits in an address.
 *
 * @param order Order to query.
 * @return Width in bits of order bits.
 */
#define order_width(order) (__mm_widths[order])

/**
 * Get size of order, as in how many pages of one order lower it can contain.
 *
 * @param order Order to query.
 * @return Number of pages of one order lower this order can contain.
 */
#define order_size(order) (__mm_sizes[order])

/**
 * Get highest order supported by the current configuration.
 *
 * @return Max supported order.
 */
#define max_order() (__mm_max_order)

/**
 * Get base page shift.
 *
 * @return Page shift.
 */
#define page_shift() (__mm_page_shift)

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
#define __va(x) (void *)(((uintptr_t)(x)) + VM_DMAP - RAM_BASE)

/**
 * Convert virtual address to physical address in direct mapping.
 *
 * @param x Virtual address.
 * @return Corresponding physical address.
 */
#define __pa(x) (void *)(((uintptr_t)(x)) - VM_DMAP + RAM_BASE)

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
#define MR_USED (1 << 8)
/** Don't free memory on clear. */
#define MR_KEEP (1 << 9)

/** @} */

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

/** Gives access to global page order shift information. \global */
extern size_t __mm_shifts[NUM_ORDERS];

/** Gives access to global page order width information. \global */
extern size_t __mm_widths[NUM_ORDERS];

/** Gives access to global page order size information. \global */
extern size_t __mm_sizes[NUM_ORDERS];

/** Gives access to global base page shift. \global */
extern size_t __mm_page_shift;

/** Gives access to global maximum order size. \global */
extern enum mm_order __mm_max_order;

/**
 * Find nearest order to size.
 *
 * @param size Size to map to an order.
 * @return Nearest order larger than \p size.
 */
enum mm_order nearest_order(size_t size);

/**
 * Initialize memory subsystem data. Populates __mm_* with data given.
 *
 * @param max_order Maximum order the current system supports.
 * @param shifts Offsets to start of each memory order in address.
 * @param page_shift Width in bits of base page size.
 * \todo Should likely also be stat_t?
 */
void init_mem(size_t max_order, size_t shifts[10], size_t page_shift);


/** Base page size. */
#define BASE_PAGE_SIZE (order_size(BASE_PAGE))

/** Base page order. */
#define BASE_PAGE (MM_O0)

#endif /* KMI_MEM_H */
