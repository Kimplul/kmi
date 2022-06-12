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
	/* yes, slightly weird situation where cur_tcb() call cpu_id() which
	 * gets the current tcb and returns the cpu id in the tcb, so that that
	 * id can be used to get the tcb we want. I might try to work out
	 * something slightly smarter, although this is likely not going to have
	 * basically any kind of importance for perfomance. */
	struct tcb *t;
	__asm__ volatile ("mv %0, tp\n" : "=r" (t) ::);
	return t->cpu_id;
}
