/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_CSR_H
#define APOS_CSR_H

/**
 * @file csr.h
 * riscv64-specific header for CSR handling.
 */

/** @name satp register memory modes. */
/** @{ */

/** Value of Sv32 mode in \c satp register. */
#define SATP_MODE_Sv32 0x80000000

/** Value of Sv39 mode in \c satp register. */
#define SATP_MODE_Sv39 0x8000000000000000

/** Value of Sv48 mode in \c satp register. */
#define SATP_MODE_Sv48 0x9000000000000000

/** @} */

/** @name Unprivileged CSR registers. */
/** @{ */

/** Address of \c time CSR. */
#define CSR_TIME 0xc01

/** Address of \c timeh CSR. */
#define CSR_TIMEH 0xc81

/** @} */

/** @name Supervisor CSR registers. */
/** @{ */

/** Address of \c sstatus CSR. */
#define CSR_SSTATUS 0x100

/** Address of \c sie CSR. */
#define CSR_SIE 0x104

/** Address of \c stvec CSR. */
#define CSR_STVEC 0x105

/** Address of \c scounteren CSR. */
#define CSR_SCOUNTEREN 0x106

/** Address of \c senvcfg CSR. */
#define CSR_SENVCFG 0x10A

/** Address of \c sscratch CSR. */
#define CSR_SSCRATCH 0x140

/** Address of \c sepc CSR. */
#define CSR_SEPC 0x141

/** Address of \c scause CSR. */
#define CSR_SCAUSE 0x142

/** Address of \c stval CSR. */
#define CSR_STVAL 0x143

/** Address of \c sip CSR. */
#define CSR_SIP 0x144

/** Address of \c satp CSR. */
#define CSR_SATP 0x180

/** Address of \c scontext CSR. */
#define CSR_SCONTEXT 0x5A8

/** @} */

/** @name Exception causes. */
/** @{ */

/** Misaligned instruction. */
#define EXC_INST_MISALIGNED 0

/** Illegal instruction access. */
#define EXC_INST_ACCESS 1

/** Hardware breakpoint. */
#define EXC_BREAKPOINT 3

/** Illegal load address. */
#define EXC_LOAD_ACCESS 5

/** Illegal store address. */
#define EXC_STORE_ACCESS 7

/** System call. */
#define EXC_SYSCALL 8

/** Instruction page fault. */
#define EXC_INST_PAGE_FAULT 12

/** Load page fault. */
#define EXC_LOAD_PAGE_FAULT 13

/** Store page fault. */
#define EXC_STORE_PAGE_FAULT 15

/** @} */

/** @name Status CSR bits. */
/** @{ */

/** \c sstatus \c SUM bit. */
#define SSTATUS_SUM (1 << 18)

/** \c sstatus \c SPP bit. */
#define SSTATUS_SPP (1 << 8)

/** @} */

/**
 * Helper macro for compiler vs. assembler string generation.
 *
 * Directly lifted from Linux:/arch/riscv/include/asm/asm.h:9-13.
 *
 * @param x Expression to be stringified.
 */
#if defined(__ASSEMBLY__)
#define __ASM_STR(x) x
#else
#define __ASM_STR(x) #x
#endif

/**
 * Read from CSR.
 *
 * @param csr Name of CSR.
 * @param res Location to where result should be written.
 */
#define csr_read(csr, res) \
	__asm__ volatile ("csrr %0, " __ASM_STR(csr) : "=r" (res) : : "memory")

/**
 * Write to CSR.
 *
 * @param csr Name of CSR.
 * @param val Value to be written.
 */
#define csr_write(csr, val) \
	__asm__ volatile ("csrw " __ASM_STR(csr) ", %0" : : "r" (val) : "memory")

/**
 * Set bits in CSR.
 *
 * @param csr Name of CSR.
 * @param val Mask of bits to set.
 */
#define csr_set(csr, val) \
	__asm__ volatile ("csrs " __ASM_STR(csr) ", %0" : : "r" (val) : "memory")

/**
 * Clear bits in CSR.
 *
 * @param csr Name of CSR.
 * @param val Mask of bits to clear.
 */
#define csr_clear(csr, val) \
	__asm__ volatile ("csrc " __ASM_STR(csr) ", %0" : : "r" (val) : "memory")

#endif /* APOS_CSR_H */
