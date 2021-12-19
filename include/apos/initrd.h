#ifndef APOS_INITRD_H
#define APOS_INITRD_H

#include <apos/types.h>
#include <apos/pmem.h>

size_t get_init_size(void *fdt);
void move_init(void *fdt, void *target);

pm_t get_initrdtop(void *fdt);
pm_t get_initrdbase(void *fdt);

#endif /* APOS_INITRD_H */
