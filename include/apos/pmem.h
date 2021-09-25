#ifndef APOS_PMEM_H
#define APOS_PMEM_H

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

typedef size_t paddr_t;
typedef ssize_t pnum_t;

void update_pmap(paddr_t offset);
void free_page(enum mm_order_t order, paddr_t paddr);
void mark_used(enum mm_order_t order, paddr_t paddr);
paddr_t alloc_page(enum mm_order_t order, paddr_t offset);

#if defined(INIT)
paddr_t populate_pmap(paddr_t ram_base, size_t ram_size, paddr_t cont);
paddr_t probe_pmap(paddr_t ram_base, size_t ram_size);
#endif

#if defined(KERNEL)
void init_pmap(void *p);
#endif

#endif /* APOS_PMEM_H */
