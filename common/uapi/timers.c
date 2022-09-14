/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file timers.c
 * Timer syscall implementations.
 */

#include <apos/timer.h>
#include <apos/uapi.h>

#include <arch/timer.h>

/**
 * Convert arch-specific register values \p ticks and \p mult to \ref ticks_t.
 *
 * If we're on a 32bit system, one register can't contain a tick value,
 * so we use two registers and combine them into one value and let the compiler
 * handle the rest.
 *
 * @param ticks Register width tick value.
 * @param mult Register width repeat value.
 * @return Corresponding \ref ticks_t value.
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
 * @return \ref OK and resolution of system timer in Hz if on 64bit system,
 * otherwise high bits of resolutio in first register and low bits in second.
 */
SYSCALL_DEFINE0(timebase)()
{
	ticks_t t = secs_to_ticks(1);
#if defined(_LP64)
	return (struct sys_ret){ OK, t };
#else
	return (struct sys_ret){ t >> 32, t };
#endif
}

/**
 * Current ticks syscall handler.
 *
 * Note that most platforms allow user level read access to hardware timers, and
 * should be preferred over this syscall on such platforms. Still, for
 * completeness sake.
 *
 * @return \ref OK and the current ticks when on 64bit systems, otherwise high
 * 32 bits of ticks in first register and low 32 bits in second.
 * \todo 32bit systems are arguably a bit unsafe with this method, should I
 * instead provide ticks_low and ticks_high or something?
 */
SYSCALL_DEFINE0(ticks)()
{
	ticks_t t = current_ticks();
#if defined(_LP64)
	return (struct sys_ret){ OK, t };
#else
	return (struct sys_ret){ t >> 32, t };
#endif
}

/**
 * Relative timer request syscall handler.
 *
 * \note On 64bit systems, \p mult is ignored as \p ticks register is large
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
