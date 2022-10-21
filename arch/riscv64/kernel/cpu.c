/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file cpu.c
 * riscv64 implementation of cpu handling.
 */

#include <apos/tcb.h>
#include <arch/cpu.h>

void cpu_assign(struct tcb *t)
{
	__asm__ volatile ("mv tp, %0\n" : : "r" (t) :);
}

id_t cpu_id()
{
	struct tcb *t = cur_tcb();
	return t->cpu_id;
}

/* override cur_tcb() in common/tcb.c since on risc-v this is apparently the
 * optimal strategy. */
/* @todo should this be the default for all arches? */
struct tcb *cur_tcb()
{
	register struct tcb *t __asm__ ("tp");
	return t;
}
