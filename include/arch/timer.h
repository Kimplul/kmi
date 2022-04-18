#ifndef APOS_ARCH_TIMER_H
#define APOS_ARCH_TIMER_H

#include <apos/timer.h>

/* return hardware timer frequency */
ticks_t stat_timer();

/* set up timer interrupt for absolute ticks */
void set_timer(ticks_t ticks);

/* current ticks */
ticks_t current_ticks();

#endif /* APOS_ARCH_TIMER_H */
