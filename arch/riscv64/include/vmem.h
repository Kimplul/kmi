/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

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

/**
 * Number of entries in one page table, depends on if we're running riscv32 or
 * riscv64.
 */
#if __riscv_xlen == 64
/** When running riscv64, each page table has 512 8byte entries. */
#define RISCV_NUM_LEAVES 512
#else
/** When running riscv32, each page table has 1024 4byte entries. */
#define RISCV_NUM_LEAVES 1024
#endif

/** Page is active. */
#define VM_V (1 << 0)

/** Page is readable. */
#define VM_R (1 << 1)

/** Page is writable. */
#define VM_W (1 << 2)

/** Page is executable. */
#define VM_X (1 << 3)

/** Page is user-accessible. */
#define VM_U (1 << 4)

/** Page is global. */
#define VM_G (1 << 5)

/** Page has been accessed. */
#define VM_A (1 << 6)

/** Page is dirty. */
#define VM_D (1 << 7)

/** Memory mode of cpu. Currently only Sv39 is supported. */
enum mm_mode {
	/** 48bit effective addresses on 64bit systems. */
	Sv48,
	/** 39bit effective addresses on 64bit systems. */
	Sv39,
	/** 32bit effective addresses on 32bit systems. */
	Sv32,
};

/**
 * Virtual memory.
 *
 * On riscv, all page tables are luckily identical, so we can
 * use this without much trickery.
 */
struct vmem {
	/** RISCV_NUM_LEAVES pointers to other page tables. */
	struct vmem *leaf[RISCV_NUM_LEAVES];
};

#endif /* APOS_RISCV_VMAP_H */
