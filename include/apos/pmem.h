#ifndef APOS_PMEM_H
#define APOS_PMEM_H

#include <apos/mem.h>
#include <apos/types.h>

/* arch */
int arch_pmem_conf(void *fdt, size_t *max_order, size_t *base_bits, size_t bits[ORDERS_NUM]);

/* common */
void update_pmap(pm_t offset);
void free_page(enum mm_order order, pm_t paddr);
void mark_used(enum mm_order order, pm_t paddr);
pm_t alloc_page(enum mm_order order, pm_t offset);

pm_t populate_pmap(pm_t ram_base, size_t ram_size, pm_t cont);
pm_t probe_pmap(pm_t ram_base, size_t ram_size);

void init_pmem(void *fdt);

#endif /* APOS_PMEM_H */
