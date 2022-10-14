/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_MEM_H
#define APOS_MEM_H

/**
 * @file mem.h
 * Generic memory, common to both physical and virtual memory.
 */

#include <apos/utils.h>
#include <apos/types.h>

/** Helper macro for getting bit width of \ref mm_info_t. */
#define MM_OINFO_WIDTH (sizeof(mm_info_t) * 8)

/**
 * Extract index of page order \c order from base page index \c pnum.
 *
 * @param pnum Base page order index.
 * @param order Order page index to convert to.
 * @return Index of page order \c order.
 */
#define pnum_to_index(pnum, order) \
	(((pnum) >> order_offset(order)) & (order_width(order) - 1))

/**
 * Convert physical memory address \c paddr to index of page order \c order.
 *
 * @param paddr Physical memory address.
 * @param order Order page index to convert to.
 * @return Index of page order \c order.
 */
#define pm_to_index(paddr, order) \
	(pnum_to_index(pm_to_pnum(paddr), (order)))

/**
 * Convert physical memory address \c paddr to corresponding page number.
 *
 * @param paddr Physical memory address.
 * @return Corresponding page number.
 */
#define pm_to_pnum(paddr) ((paddr) >> __mm_page_shift)

/**
 * Convert page number to physical address.
 * Note that since a page number is the base page an address lies in,
 * @code pnum_to_pm(pm_to_pnum(p)) != p @endcode
 *
 * @param pnum Page number.
 * @return Corresponding physical address.
 */
#define pnum_to_pm(pnum) ((pnum) << __mm_page_shift)

/**
 * Add \c num to \c var and return value before addition.
 *
 * @param var Variable to add \c num to.
 * @param num Number to add to \c var.
 * @return Value of \c var before addition.
 */
#define move_forward(var, num) (((var) += (num)) - (num))

/**
 * Helper for calculating highest index of elements in order info map.
 * Since the number of entries is stored with the granularity of \c MM_OINFO_WIDTH,
 * the highest index element is \c num rounded up to the nearest index multiple
 * of \c MM_OINFO_WIDTH. This is due to some data access optimizations over in
 * common/pmem.c.
 *
 * @param num Number of entries in map.
 * @return Highest index of element in map.
 */
#define num_elems(num) \
	(((num) + MM_OINFO_WIDTH - 1) / MM_OINFO_WIDTH)
/**
 * Helper for calculating starting index of element.
 *
 * @param num Number of starting entry.
 * @return Index of starting element in which entry resides.
 */
#define num_indexes(num) ((num) / MM_OINFO_WIDTH)

/**
 * Helper for calculating index of element from entry number.
 *
 * @param num Entry number.
 * @return Index of element in which entry resides.
 */
#define index_elems(num) ((num) / MM_OINFO_WIDTH)

/**
 * Helper for calculating size of state for storing elements in.
 *
 * @param num Number of entries.
 * @return Size of element state buffer.
 */
#define state_elems(num) (sizeof(mm_info_t) * (num_elems(num)))

/**
 * Helper for calculating size of pointer buffer.
 *
 * @param num Number of entries.
 * @return Size of pointer buffer.
 */
#define next_elems(num) (sizeof(void *) * (num))

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
#define order_offset(order) (__mm_shifts[order])

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

/**
 * Entry index within the element that contains it.
 *
 * @param idx Index of entry.
 * @return Index of entry within its containing element.
 */
#define order_bit(idx) ((idx) & (MM_OINFO_WIDTH - 1))

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
 * \todo Isn't this the same as \ref pm_to_pnum()?
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
/** Memory region is shared. */
#define MR_SHARED (1 << 9)
/** Owner of shared region. */
#define MR_OWNED (1 << 10)
/** Don't free memory on clear. */
#define MR_KEEP (1 << 11)

/** @} */

/** Maximum number of page orders allowed. Likely massively overkill. */
#define NUM_ORDERS 10

/** Gives access to global page order shift information. \global */
extern size_t __mm_shifts[NUM_ORDERS];

/** Gives access to global page order width information. \global */
extern size_t __mm_widths[NUM_ORDERS];

/** Gives access to global page order size information. \global */
extern size_t __mm_sizes[NUM_ORDERS];

/** Gives access to global base page shift. \global */
extern size_t __mm_page_shift;

/** Gives access to global maximum order size. \global */
extern size_t __mm_max_order;

/** Give names to page orders. */
enum mm_order {
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
};

/** Page number. */
typedef ssize_t pnum_t;

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

#endif /* APOS_MEM_H */
