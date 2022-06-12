/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file timers.c
 * Timer syscall implementations.
 */

#include <apos/timer.h>
#include <apos/uapi.h>

static ticks_t scaled_ticks(sys_arg_t ticks, sys_arg_t repeat)
{
#if __WORDSIZE == 64
	UNUSED(repeat);
	return ticks;
#else
	return ((ticks_t)ticks << 32) + repeat;
#endif
}

SYSCALL_DEFINE0(timebase)()
{
	return (struct sys_ret){ OK, secs_to_ticks(1) };
}

SYSCALL_DEFINE2(req_rel_timer)(sys_arg_t ticks, sys_arg_t repeat)
{
	return (struct sys_ret){
		       OK,
		       new_rel_timer(cur_tcb()->tid,
		                     scaled_ticks(ticks, repeat))
	};
}

SYSCALL_DEFINE2(req_abs_timer)(sys_arg_t ticks, sys_arg_t repeat)
{
	return (struct sys_ret){
		       OK,
		       new_abs_timer(cur_tcb()->tid,
		                     scaled_ticks(ticks, repeat))
	};
}

SYSCALL_DEFINE1(free_timer)(sys_arg_t cid)
{
	struct timer *timer = find_timer(cid);
	if (!timer)
		return (struct sys_ret){ ERR_NF, 0 };

	remove_timer(timer);
	return (struct sys_ret){ OK, 0 };
}
