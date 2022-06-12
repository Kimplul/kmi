/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_ARCH_TIMER_H
#define APOS_ARCH_TIMER_H

/**
 * @file timer.h
 * Arch-specific timer handling, generally implemented in
 * arch/whatever/timer.c
 */

#include <apos/timer.h>

#if defined(riscv64)
#include "../../arch/riscv64/include/timer.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/timer.h"
#endif

/**
 * Get hardware timer frequency.
 *
 * @param fdt Global FDT pointer.
 * @return Hardware timer frequency, ticks/sec.
 */
ticks_t stat_timer(const void *fdt);

/**
 * Set up timer interrupt for absolute ticks.
 *
 * @param ticks Time point for timer to trigger.
 * \todo Should maybe be stat_t?
 */
void set_timer(ticks_t ticks);

/**
 * Get current ticks.
 *
 * @return Current tick count.
 */
ticks_t current_ticks();

#endif /* APOS_ARCH_TIMER_H */
