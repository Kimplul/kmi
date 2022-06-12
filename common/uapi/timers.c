/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file timers.c
 * Timer syscall implementations.
 */

#include <apos/timer.h>
#include <apos/uapi.h>

/**
 * Convert arch-specific register values \c ticks and \c repeat to \c ticks_t.
 *
 * If we're on a 32bit system, one register can't contain a tick value,
 * so we use two registers and combine them into one value and let the compiler
 * handle the rest.
 *
 * @param ticks Register width tick value.
 * @param mult Register width repeat value.
 * @return Corresponding \c ticks_t value.
 */
static ticks_t scaled_ticks(sys_arg_t ticks, sys_arg_t mult)
{
#if defined(_LP64)
	UNUSED(mult);
	return ticks;
#else
	return (ticks_t)ticks * (ticks_t)mult;
#endif
}

/**
 * Timebase syscall handler.
 *
 * @return \ref OK and resolution of system timer in Hz.
 */
SYSCALL_DEFINE0(timebase)()
{
	return (struct sys_ret){ OK, secs_to_ticks(1) };
}

/**
 * Relative timer request syscall handler.
 *
 * \note On 64bit systems, \c repeat is ignored as \c ticks register is large
 * enough to contain essentially any timepoint we want. A couple thousand years
 * when the clock runs at 5GHz, if I'm not completely mistaken.
 *
 * @param ticks Number of ticks from now.
 * @param mult Multiply \c ticks by this value.
 * @return \ref OK and \c cid of created timer.
 */
SYSCALL_DEFINE2(req_rel_timer)(sys_arg_t ticks, sys_arg_t mult)
{
	return (struct sys_ret){
		       OK,
		       new_rel_timer(cur_tcb()->tid,
		                     scaled_ticks(ticks, mult))
	};
}

/**
 * Absolute timer request syscall handler.
 *
 * @param ticks Absolute timepoint relative to some start point defined at boot.
 * @param mult Multiply \c ticks by this value.
 * @return \ref OK and \c cid of created timer.
 * \see req_rel_timer().
 */
SYSCALL_DEFINE2(req_abs_timer)(sys_arg_t ticks, sys_arg_t mult)
{
	return (struct sys_ret){
		       OK,
		       new_abs_timer(cur_tcb()->tid,
		                     scaled_ticks(ticks, mult))
	};
}

/**
 * Free timer request syscall handler.
 *
 * @param cid \c cid of timer to free.
 * @return \ref ERR_NF and \c 0if no timer could be found with \c cid, \ref OK
 * and 0 otherwise.
 */
SYSCALL_DEFINE1(free_timer)(sys_arg_t cid)
{
	struct timer *timer = find_timer(cid);
	if (!timer)
		return (struct sys_ret){ ERR_NF, 0 };

	remove_timer(timer);
	return (struct sys_ret){ OK, 0 };
}
