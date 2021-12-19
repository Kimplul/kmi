#ifndef APOS_PMEM_H
#define APOS_PMEM_H

#include <apos/mem.h>
#include <apos/types.h>

enum mm_order_t {
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

typedef size_t pm_t;
typedef ssize_t pnum_t;

void update_pmap(pm_t offset);
void free_page(enum mm_order_t order, pm_t paddr);
void mark_used(enum mm_order_t order, pm_t paddr);
pm_t alloc_page(enum mm_order_t order, pm_t offset);

pm_t populate_pmap(pm_t ram_base, size_t ram_size, pm_t cont);
pm_t probe_pmap(pm_t ram_base, size_t ram_size);

#endif /* APOS_PMEM_H */
