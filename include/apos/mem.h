#ifndef APOS_MEM_H
#define APOS_MEM_H

#include <apos/utils.h>
#include <pages.h>

enum mm_mode_t {
	Sv48,
	Sv39,
	Sv32,
};

#define MM_OINFO_WIDTH (sizeof(mm_info_t) * 8)

#define pnum_to_index(pnum, order) (((pnum) >> __o_offset(order)) & (__o_width(order) - 1))
#define pm_to_index(paddr, order) (pnum_to_index(pm_to_pnum(paddr), (order)))
#define pm_to_pnum(paddr) ((paddr) >> __mm_page_shift)
#define pnum_to_paddr(pnum) ((pnum) << __mm_page_shift)

#define move_forward(var, num) (((var) += (num)) - (num))

#define move_paddr(paddr, base, offset) ((((pm_t)(paddr)) - (base)) + (offset))
#define num_elems(num) (((num) + MM_OINFO_WIDTH - 1) / MM_OINFO_WIDTH)
#define num_indexes(num) ((num) / MM_OINFO_WIDTH)
#define index_elems(num) ((num) / MM_OINFO_WIDTH)
#define state_elems(num) (sizeof(mm_info_t) * (num_elems(num)))
#define next_elems(num) (sizeof(void *) * (num))
#define max_index(order) (__o_width(order) - 1)

#define __o_offset(order) (__mm_shifts[order])
#define __o_width(order) (__mm_widths[order])
#define __o_elems(order) (__mm_widths[order] / MM_OINFO_WIDTH)

#define __o_container(idx) ((idx) / MM_OINFO_WIDTH)
#define __o_bit(idx) ((idx) & (MM_OINFO_WIDTH - 1))

extern size_t __mm_shifts[10];
extern size_t __mm_widths[10];
extern size_t __mm_sizes[10];

extern size_t __mm_page_shift;
extern size_t __mm_max_order;

void init_mem(size_t max_order, size_t shifts[10], size_t page_shift);
enum mm_mode_t get_mmode(void *fdt);

#endif /* APOS_MEM_H */
