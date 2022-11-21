/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file cpu.c
 * riscv64 implementation of cpu handling.
 */

#include <apos/tcb.h>
#include <apos/atomic.h>

#include <arch/cpu.h>

#include "sbi.h"

/** Keeps track of initialized cpus. */
static atomic_long cpus = 0;

void cpu_assign(struct tcb *t)
{
	/* bounce cpu id forward, or if first time here, get a cpu id */
	struct tcb *c = cur_tcb();
	if (likely(c))
		t->cpu_id = c->cpu_id;
	else
		t->cpu_id = cpus++;

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

/**
 * Helper for cpu_send_ipi() to convert linear \p cpu_id to SBI \c
 * hart_mask_base.
 *
 * @param cpu_id CPU id to convert.
 * @return Corresponding \c hart_mask_base.
 */
static long __cpu_offset(id_t cpu_id)
{
	return cpu_id / (sizeof(long) * 8);
}

/**
 * Helper for cpu_send_ipi() to convert linear \p cpu_id to SBI \c
 * hart_mask.
 *
 * @param cpu_id CPU id to convert.
 * @param offset Offset from __cpu_offset().
 * @return Corresponding \c hart_mask.
 */
static long __cpu_mask(id_t cpu_id, long offset)
{
	return cpu_id - offset * sizeof(long) * 8;
}

void cpu_send_ipi(id_t cpu_id)
{
	long offset = __cpu_offset(cpu_id);
	long mask = __cpu_mask(cpu_id, offset);
	sbi_send_ipi(mask, offset);
}
