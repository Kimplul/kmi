/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_RISCV_CONFIG_H
#define APOS_RISCV_CONFIG_H

/**
 * @file config.h
 * riscv64 specific configuration parameters, currently mixing user-specified
 * and immutable parameters. Included on the command line by
 * -include.
 *
 * Assume Sv39, probably wouldn't be too difficult to use runtime parameters
 * instead. First 4K is reserved for NULL, but I suppose it could be mapped
 * later if *absolutely* necessary.
 *
 * \todo Consider separating user-specified and immutable parameters.
 * \todo Write more thorough documentation, maybe a separate .md file?
 */

#include <apos/sizes.h>

/* --- START ARCH USER CONFIG VALUES --- */
/** Physical address to where the OS image will be loaded. */
#define RAM_BASE 0x80000000
/* --- END ARCH USER CONFIG VALUES --- */

/* don't touch >:( */

/**
 * How much space to reserve for possible firmware region.
 * Assumed to be at start of RAM.
 */
#define FW_MAX_SIZE (SZ_2M)

/** Maximum size of the kernel proper. Largely arbitrary. */
/** \todo UBSAN is getting pretty close to this limit, should it be raised? */
#define PM_KERN_SIZE (SZ_256K)

/** Physical address to where the kernel proper will be relocated. */
#define PM_KERN_BASE (RAM_BASE + FW_MAX_SIZE + PM_KERN_SIZE)

/** Highest allowed physical address where kernel stuff may lie. */
#define PM_KERN_TOP (PM_KERN_BASE + PM_KERN_SIZE)

/** Physical memory stack base. In this case, right after the kernel. */
#define PM_STACK_BASE (PM_KERN_BASE + PM_KERN_SIZE)

/** Size of physical memory stack. */
#define PM_STACK_SIZE (SZ_256K)

/** Top of physical memory stack. */
#define PM_STACK_TOP (PM_STACK_BASE + PM_STACK_SIZE)

#if defined(riscv64)
/* 64bit */

/** Direct map offset. */
#define VM_DMAP (0xffffffc000000000)         /* testing for now */

/** Virtual address of kernel proper inside direct mapping. */
#define VM_KERN (VM_DMAP + FW_MAX_SIZE)

/** Page reserved for kernel I/O. */
#define IO_PAGE 511UL

/** Direct mapping starts from this page. */
#define KSTART_PAGE 256UL

/** The RPC stack page. */
#define CSTACK_PAGE 255UL

/** User virtual memory space start. */
#define UVMEM_START (SZ_4K)

/**  User virtual memory space end. */
#define UVMEM_END (SZ_256G - SZ_1G)

/** RPC stack top. */
#define RPC_STACK_TOP (SZ_256G)

/** RPC stack base. */
#define RPC_STACK_BASE (SZ_256G - SZ_1G)

#else
/* 32bit */

/** \todo figure this stuff out */
#define VM_DMAP (0x000000000)
#define VM_KERN (VM_DMAP + SZ_256K)
#define ROOT_PTE (0UL)
#define ROOT_REGION (SZ_4K)

#define IO_PAGE 1023UL
#define KSTART_PAGE 512UL
#define CSTACK_PAGE 511UL

#define UVMEM_START (SZ_4K)
#define UVMEM_END (SZ_4G - SZ_8M)

#define PROC_STACK_TOP (SZ_4G)
#define PROC_STACK_BASE (SZ_4G - SZ_8M)
#endif
#endif /* APOS_RISCV_CONFIG_H */
