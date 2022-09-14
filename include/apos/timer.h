/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_TIMER_H
#define APOS_TIMER_H

/**
 * @file timer.h
 * Timer handling.
 *
 * \todo Document exceptions and return values better.
 */

#include <apos/types.h>

/**
 * ticks_t typedef, use unsigned 64bit integer on all platforms.
 *
 * GCC will compile uint64_t even on 32bit platforms, just with some runtime
 * overhead, should be fine. This will allow us to have a reasonable time range
 * even with nanosecond clocks. (138 years with ~4.2 Hz clock)
 */
typedef uint64_t ticks_t;

/**
 * tunit_t typedef, whichever time unit we're dealing with.
 */
typedef size_t tunit_t;

/**
 * Timer structure.
 */
struct timer {
	/** tid. Thread ID of whoever scheduled the timer. */
	id_t tid;
	/** cid. Control ID, used to differentiate timers. */
	id_t cid;

	/** ticks. Absolute number of ticks, essentially a timepoint for when
	 * the timer should trigger. */
	ticks_t ticks;
};

/**
 * Initialize timers.
 *
 * @param fdt Pointer to global FDT
 */
void init_timer(const void *fdt);

/**
 * Set up timer interrupt ticks from now.
 *
 * @param tid Thread id for callback.
 * @param ticks Ticks from \ref current_ticks().
 * @return Id of created timer.
 */
id_t new_rel_timer(id_t tid, ticks_t ticks);

/**
 * Set up timer interrupt at ticks.
 *
 * @param tid Thread id for callback.
 * @param ticks Ticks from \ref current_ticks().
 * @return Id of created timer.
 */
id_t new_abs_timer(id_t tid, ticks_t ticks);

/**
 * Return a pointer to the newest timer, i.e. the one that is closest to
 * triggering.
 *
 * @return Pointer to a timer or NULL if queue is empty.
 */
struct timer *newest_timer();

/**
 * Find a timer associated with a specific control ID.
 *
 * @param cid Control ID to find.
 * @return Pointer to associated timer if found, else NULL.
 */
struct timer *find_timer(id_t cid);

/**
 * Remove a timer.
 *
 * @param timer Pointer to timer to remove.
 * @return OK on success.
 */
stat_t remove_timer(struct timer *timer);

/**
 * Convert nanoseconds to ticks.
 *
 * @param nsecs Number of nanoseconds to represent as ticks.
 * @return Equivalent ticks to nsecs.
 */
ticks_t nsecs_to_ticks(tunit_t nsecs);

/**
 * Convert microseconds to ticks.
 *
 * @param usecs Number of microseconds to represent as ticks.
 * @return Equivalent ticks to usecs.
 */
static inline ticks_t usecs_to_ticks(tunit_t usecs)
{
	return nsecs_to_ticks(usecs * 1000);
}

/**
 * Convert milliseconds to ticks.
 *
 * @param msecs Number of milliseconds to represent as ticks.
 * @return Equivalent ticks to msecs.
 */
static inline ticks_t msecs_to_ticks(tunit_t msecs)
{
	return usecs_to_ticks(msecs * 1000);
}

/**
 * Convert seconds to ticks.
 *
 * @param secs Number of seconds to represent as ticks.
 * @return Equivalent ticks to secs.
 */
static inline ticks_t secs_to_ticks(tunit_t secs)
{
	/** \todo likely not a problem on 64bit systems, not sure how to handle situation on
	 * 32bit */
	return msecs_to_ticks(secs * 1000);
}

#endif /* APOS_TIMER_H */
