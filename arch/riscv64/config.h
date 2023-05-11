/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_RISCV_CONFIG_H
#define KMI_RISCV_CONFIG_H

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

#include <kmi/sizes.h>

/* --- START ARCH USER CONFIG VALUES --- */
/** Physical address to where the OS image will be loaded. */
//#define RAM_BASE 0x80000000
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

/** Virtual memory stack base. In this case, right after the kernel. */
#define VM_STACK_BASE (VM_KERN + PM_KERN_SIZE)

/** Size of virtual memory stack. */
#define VM_STACK_SIZE (SZ_4K)

/** Top of virtual memory stack. */
#define VM_STACK_TOP (VM_STACK_BASE + VM_STACK_SIZE)

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

/**
 * The RPC stack page.
 * @todo this should be page before KSTART, but currently
 * RPC_STACK_TOP is too low (why? was there a reason to place it so low?) and overlaps.
 */
#define CSTACK_PAGE 248UL

/** User virtual memory space start. */
#define UVMEM_START (SZ_4K)

/**  User virtual memory space end. */
#define UVMEM_END (SZ_256G - SZ_8G)

/** RPC stack top. */
#define RPC_STACK_TOP (UVMEM_END + SZ_1G)

/** RPC stack base. */
#define RPC_STACK_BASE (UVMEM_END)

/**
 * Size of the default top page.
 * For now we're only targeting Sv39, so this can easily be a macro.
 * Eventually it might have to be turned into a function call to support Sv48
 * etc.
 */
#define TOP_PAGE_SIZE SZ_1G

/** Default Sv mode on rv64 in kmi. */
#define DEFAULT_Sv_MODE Sv39

#else
/* 32bit */

/** \todo figure this stuff out */
#define VM_DMAP 0x80000000 /* works on qemu at least */
#define VM_KERN (VM_DMAP + FW_MAX_SIZE)

#define IO_PAGE 1023UL
#define KSTART_PAGE 512UL
#define CSTACK_PAGE 511UL

#define UVMEM_START (SZ_4K)
#define UVMEM_END (SZ_2G - SZ_8M)

#define RPC_STACK_TOP (UVMEM_END)
#define RPC_STACK_BASE (RPC_STACK_TOP - SZ_8M)

#define TOP_PAGE_SIZE SZ_4M
#define DEFAULT_Sv_MODE Sv32

#endif
#endif /* KMI_RISCV_CONFIG_H */
