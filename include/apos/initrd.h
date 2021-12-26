#ifndef APOS_INITRD_H
#define APOS_INITRD_H

#include <apos/types.h>
#include <apos/pmem.h>
#include <apos/vmem.h>

size_t get_init_size(void *fdt);
vm_t get_init_base(void *fdt);

pm_t get_initrdtop(void *fdt);
pm_t get_initrdbase(void *fdt);

#endif /* APOS_INITRD_H */
