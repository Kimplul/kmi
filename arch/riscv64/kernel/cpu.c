/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file cpu.c
 * riscv64 implementation of cpu handling.
 */

#include <kmi/tcb.h>
#include <kmi/atomic.h>

#include <arch/cpu.h>

#include "sbi.h"

void cpu_assign(struct tcb *t)
{
	/* bounce cpu id forward */
	struct tcb *c = cur_tcb();
	t->cpu_id = c->cpu_id;
	__asm__ volatile ("mv tp, %0\n" : : "r" (t) :);
}

void tcb_assign(struct tcb *t)
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
	struct tcb *t;
	__asm__ volatile ("mv %0, tp" : "=r" (t) ::);
	return t;
}

void cpu_send_ipi(id_t cpu_id)
{
	sbi_send_ipi(1, cpu_id);
}
