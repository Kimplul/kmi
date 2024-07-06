/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_RISCV_VMAP_H
#define KMI_RISCV_VMAP_H

#include "uapi.h"

/**
 * @file vmem.h
 * riscv64 definitions of arch-specific virtual memory data types and macros.
 * Very likely VM_* should be moved to \ref include/kmi/vmem.h and made
 * architecture-nonspecific, but works for now.
 */

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

/**
 * Extract virtual memory flags (MR_XXX).
 *
 * @param x Flags to extract virtual memory region flags from.
 * @return Virtual memory region flags.
 */
#define vm_flags(x) ((x) & ~0xff)

/**
 * Extract physical memory page flags (VM_XXX).
 *
 * @param x Flags to extract physical memory page flags from.
 * @return Physical memory page flags.
 */
#define vp_flags(x) ((x) & 0xff)

/** How many VM_XXX flags architecture has. MR_XXX are applied after them. */
#define ARCH_VP_FLAGS 8

#endif /* KMI_RISCV_VMAP_H */
