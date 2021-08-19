#ifndef APOS_RISCV_VMAP_H
#define APOS_RISCV_VMAP_H

#include <apos/sizes.h>

#define VM_V (1 << 0)
#define VM_R (1 << 1)
#define VM_W (1 << 2)
#define VM_X (1 << 3)
#define VM_U (1 << 4)
#define VM_G (1 << 5)
#define VM_A (1 << 6)
#define VM_D (1 << 7)

#define VM_TOP (-1)
#define VM_KERN (VM_TOP - SZ_1G)

#endif /* APOS_RISCV_VMAP_H */
