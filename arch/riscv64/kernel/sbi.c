/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file sbi.c
 * riscv64 specific OpenSBI handling.
 */

#include "sbi.h"

struct sbiret sbi_ecall(int ext, int fid, unsigned long arg0,
                        unsigned long arg1, unsigned long arg2,
                        unsigned long arg3, unsigned long arg4,
                        unsigned long arg5)
{
	struct sbiret ret;

	register uintptr_t a0 __asm__ ("a0") = (uintptr_t)(arg0);
	register uintptr_t a1 __asm__ ("a1") = (uintptr_t)(arg1);
	register uintptr_t a2 __asm__ ("a2") = (uintptr_t)(arg2);
	register uintptr_t a3 __asm__ ("a3") = (uintptr_t)(arg3);
	register uintptr_t a4 __asm__ ("a4") = (uintptr_t)(arg4);
	register uintptr_t a5 __asm__ ("a5") = (uintptr_t)(arg5);
	register uintptr_t a6 __asm__ ("a6") = (uintptr_t)(fid);
	register uintptr_t a7 __asm__ ("a7") = (uintptr_t)(ext);

	__asm__ volatile ("ecall"
	                  : "+r" (a0), "+r" (a1)
	                  : "r" (a2), "r" (a3), "r" (a4), "r" (a5), "r" (a6),
	                  "r" (a7)
	                  : "memory");

	ret.error = a0;
	ret.value = a1;

	return ret;
}
