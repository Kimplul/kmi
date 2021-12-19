#ifndef APOS_INITRD_H
#define APOS_INITRD_H

#include <apos/types.h>

size_t get_init_size(void *fdt);
void move_init(void *fdt, void *target, size_t sz);

#endif /* APOS_INITRD_H */
