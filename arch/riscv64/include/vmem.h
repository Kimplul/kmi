#ifndef APOS_RISCV_VMAP_H
#define APOS_RISCV_VMAP_H

/**
 * @file vmem.h
 * riscv64 definitions of arch-specific virtual memory data types and macros.
 * Very likely VM_* should be moved to \ref include/apos/vmem.h and made
 * architecture-nonspecific, but works for now.
 */

#include <apos/types.h>
#include <apos/attrs.h>

#if __riscv_xlen == 64
#define RISCV_NUM_LEAVES 512
#else
#define RISCV_NUM_LEAVES 1024
#endif

#define VM_V (1 << 0)
#define VM_R (1 << 1)
#define VM_W (1 << 2)
#define VM_X (1 << 3)
#define VM_U (1 << 4)
#define VM_G (1 << 5)
#define VM_A (1 << 6)
#define VM_D (1 << 7)

enum mm_mode {
	Sv48,
	Sv39,
	Sv32,
};

struct vmem {
	struct vmem *leaf[RISCV_NUM_LEAVES];
};

#endif /* APOS_RISCV_VMAP_H */
