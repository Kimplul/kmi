#ifndef APOS_MEM_H
#define APOS_MEM_H

#include <apos/utils.h>
#include <apos/types.h>

#define MM_OINFO_WIDTH (sizeof(mm_info_t) * 8)

#define pnum_to_index(pnum, order)                                             \
	(((pnum) >> __o_offset(order)) & (__o_width(order) - 1))
#define pm_to_index(paddr, order)       (pnum_to_index(pm_to_pnum(paddr), (order)))
#define pm_to_pnum(paddr)               ((paddr) >> __mm_page_shift)
#define pnum_to_paddr(pnum)             ((pnum) << __mm_page_shift)

#define move_forward(var, num)          (((var) += (num)) - (num))

#define move_paddr(paddr, base, offset) ((((pm_t)(paddr)) - (base)) + (offset))
#define num_elems(num)                  (((num) + MM_OINFO_WIDTH - 1) / MM_OINFO_WIDTH)
#define num_indexes(num)                ((num) / MM_OINFO_WIDTH)
#define index_elems(num)                ((num) / MM_OINFO_WIDTH)
#define state_elems(num)                (sizeof(mm_info_t) * (num_elems(num)))
#define next_elems(num)                 (sizeof(void *) * (num))
#define max_index(order)                (__o_width(order) - 1)

#define __o_offset(order)               (__mm_shifts[order])
#define __o_width(order)                (__mm_widths[order])
#define __o_size(order)                 (__mm_sizes[order])
#define __o_elems(order)                (__mm_widths[order] / MM_OINFO_WIDTH)

#define __o_container(idx)              ((idx) / MM_OINFO_WIDTH)
#define __o_bit(idx)                    ((idx) & (MM_OINFO_WIDTH - 1))

#define __va(x)                         (void *)(((uintptr_t)(x)) + VM_DMAP - RAM_BASE)
#define __pa(x)                         (void *)(((uintptr_t)(x)) - VM_DMAP + RAM_BASE)
#define __page(x)                       ((x) / BASE_PAGE_SIZE)
#define __addr(x)                       ((x)*BASE_PAGE_SIZE)
#define __pages(x)                                                             \
	(is_aligned((x), BASE_PAGE_SIZE) ? __page((x)) :                       \
                                           __page((x) + BASE_PAGE_SIZE))
#define __bytes(x) (__addr(x))

/* if memory region is used or not */
#define MR_USED    (1 << 8)
/* if memory region is shared */
#define MR_SHARED  (1 << 9)
/* owner of shared region */
#define MR_OWNED   (1 << 10)
/* copy on write */
#define MR_COW     (1 << 11)

extern size_t __mm_shifts[10];
extern size_t __mm_widths[10];
extern size_t __mm_sizes[10];

extern size_t __mm_page_shift;
extern size_t __mm_max_order;

#define NUM_ORDERS 10
enum mm_order {
	MM_O0,
	MM_O1,
	MM_O2,
	MM_O3,
	MM_O4,
	MM_O5,
	MM_O6,
	MM_O7,
	MM_O8,
	MM_O9,
};

typedef ssize_t pnum_t;
void init_mem(size_t max_order, size_t shifts[10], size_t page_shift);
enum mm_mode get_mmode(void *fdt);

#define BASE_PAGE_SIZE (__o_size(BASE_PAGE))
#define BASE_PAGE      (MM_O0)

#endif /* APOS_MEM_H */
