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
/* nada for now */
/* --- END ARCH USER CONFIG VALUES --- */

/* don't touch >:( */

#if __riscv_xlen == 64
/* 64bit */

/** Direct map offset. */
#define VM_DMAP (0xffffffc000000000)         /* testing for now */

/** Page reserved for kernel mapping in O1 */
#define KERNEL_PAGE 510UL

/** Kernel virtual address */
#define VM_KERNEL (0xffffffff80000000)

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

/** User virtual memory space start. Should essentially always be the start of
 * the NULL page, which is marked reserved in the address space by \ref init_uvmem().*/
#define UVMEM_START (0)

/**  User virtual memory space end. */
#define UVMEM_END (SZ_256G - SZ_8G)

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
