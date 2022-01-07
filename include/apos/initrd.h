#ifndef APOS_INITRD_H
#define APOS_INITRD_H

#include <apos/types.h>
#include <apos/pmem.h>
#include <apos/vmem.h>

size_t get_init_size(const void *fdt);
vm_t get_init_base(const void *fdt);

pm_t get_initrdtop(const void *fdt);
pm_t get_initrdbase(const void *fdt);

#endif /* APOS_INITRD_H */
