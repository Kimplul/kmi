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

/* return hardware timer frequency */
ticks_t stat_timer();

/* set up timer interrupt for absolute ticks */
void set_timer(ticks_t ticks);

/* current ticks */
ticks_t current_ticks();

#endif /* APOS_ARCH_TIMER_H */
