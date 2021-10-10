#ifndef APOS_MEM_H
#define APOS_MEM_H

#include <apos/utils.h>
#include <pages.h>

#if defined(O0_WIDTH)
#define MM_O0_SHIFT (0)
#define MM_O0_WIDTH (1UL << (O0_WIDTH))
#define MM_O0_SIZE  (1UL << MM_O0_SHIFT << PAGE_SHIFT)
#else
#define MM_O0_SHIFT 0
#define MM_O0_WIDTH 0
#define MM_O0_SIZE  0
#endif

#if defined(O1_WIDTH)
#define MM_O1_SHIFT (MM_O0_SHIFT + (O0_WIDTH))
#define MM_O1_WIDTH (1UL << (O1_WIDTH))
#define MM_O1_SIZE  (1UL << MM_O1_SHIFT << PAGE_SHIFT)
#else
#define MM_O1_SHIFT 0
#define MM_O1_SHIFT 0
#define MM_O1_SIZE  0
#endif

#if defined(O2_WIDTH)
#define MM_O2_SHIFT (MM_O1_SHIFT + (O1_WIDTH))
#define MM_O2_WIDTH (1UL << (O2_WIDTH))
#define MM_O2_SIZE  (1UL << MM_O2_SHIFT << PAGE_SHIFT)
#else
#define MM_O2_SHIFT 0
#define MM_O2_WIDTH 0
#define MM_O2_SIZE  0
#endif

#if defined(O3_WIDTH)
#define MM_O3_SHIFT (MM_O2_SHIFT + (O2_WIDTH))
#define MM_O3_WIDTH (1UL << (O3_WIDTH))
#define MM_O3_SIZE  (1UL << MM_O3_SHIFT << PAGE_SHIFT)
#else
#define MM_O3_SHIFT 0
#define MM_O3_WIDTH 0
#define MM_O3_SIZE  0
#endif

#if defined(O4_WIDTH)
#define MM_O4_SHIFT (MM_O3_SHIFT + (O3_WIDTH))
#define MM_O4_SHIFT (1UL << (O4_WIDTH))
#define MM_O4_SIZE  (1UL << MM_O4_SHIFT << PAGE_SHIFT)
#else
#define MM_O4_SHIFT 0
#define MM_O4_WIDTH 0
#define MM_O4_SIZE  0
#endif

#if defined(O5_WIDTH)
#define MM_O5_SHIFT (MM_O4_SHIFT + (O4_WIDTH))
#define MM_O5_WIDTH (1UL << (O5_WIDTH))
#define MM_O5_SIZE  (1UL << MM_O5_SHIFT << PAGE_SHIFT)
#else
#define MM_O5_SHIFT 0
#define MM_O5_WIDTH 0
#define MM_O5_SIZE  0
#endif

#if defined(O6_WIDTH)
#define MM_O6_SHIFT (MM_O5_SHIFT + (O5_WIDTH))
#define MM_O6_WIDTH (1UL << (O6_WIDTH))
#define MM_O6_SIZE  (1UL << MM_O6_SHIFT << PAGE_SHIFT)
#else
#define MM_O6_SHIFT 0
#define MM_O6_WIDTH 0
#define MM_O6_SIZE  0
#endif

#if defined(O7_WIDTH)
#define MM_O7_SHIFT (MM_O6_SHIFT + (O6_WIDTH))
#define MM_O7_WIDTH (1UL << (O7_WIDTH))
#define MM_O7_SIZE  (1UL << MM_O7_SHIFT << PAGE_SHIFT)
#else
#define MM_O7_SHIFT 0
#define MM_O7_WIDTH 0
#define MM_O7_SIZE  0
#endif

#if defined(O8_WIDTH)
#define MM_O8_SHIFT (MM_O7_SHIFT + (O7_WIDTH))
#define MM_O8_WIDTH (1UL << (O8_WIDTH))
#define MM_O8_SIZE  (1UL << MM_O8_SHIFT << PAGE_SHIFT)
#else
#define MM_O8_SHIFT 0
#define MM_O8_WIDTH 0
#define MM_O8_SIZE  0
#endif

#if defined(O9_WIDTH)
#define MM_O9_SHIFT (MM_O8_SHIFT + (O8_WIDTH))
#define MM_O9_WIDTH (1UL << (O9_WIDTH))
#define MM_O9_SIZE  (1UL << MM_O9_SHIFT << PAGE_SHIFT)
#else
#define MM_O9_SHIFT 0
#define MM_O9_WIDTH 0
#define MM_O9_SIZE  0
#endif

#define MM_OINFO_WIDTH (sizeof(mm_info_t) * 8)

#define pnum_to_index(pnum, order) (((pnum) >> __o_offset(order)) & (__o_width(order) - 1))
#define paddr_to_index(paddr, order) (pnum_to_index(paddr_to_pnum(paddr), (order)))
#define paddr_to_pnum(paddr) ((paddr) >> PAGE_SHIFT)
#define pnum_to_paddr(pnum) ((pnum) << PAGE_SHIFT)

#define move_forward(var, num) (((var) += (num)) - (num))

#define move_paddr(paddr, base, offset) ((((paddr_t)(paddr)) - (base)) + (offset))
#define num_elems(num) (((num) + MM_OINFO_WIDTH - 1) / MM_OINFO_WIDTH)
#define num_indexes(num) ((num) / MM_OINFO_WIDTH)
#define index_elems(num) ((num) / MM_OINFO_WIDTH)
#define state_elems(num) (sizeof(mm_info_t) * (num_elems(num)))
#define next_elems(num) (sizeof(void *) * (num))
#define max_index(order) (__o_width(order) - 1)

#define __o_offset(order) (mm_shifts[order])
#define __o_width(order) (mm_widths[order])
#define __o_elems(order) (mm_widths[order] / MM_OINFO_WIDTH)

#define __o_container(idx) ((idx) / MM_OINFO_WIDTH)
#define __o_bit(idx) ((idx) & (MM_OINFO_WIDTH - 1))

extern const size_t mm_shifts[10];
extern const size_t mm_widths[10];
extern const size_t mm_sizes[10];

#endif /* APOS_MEM_H */
