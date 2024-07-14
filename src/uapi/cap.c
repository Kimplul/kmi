/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023 Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#include <kmi/uapi.h>
#include <kmi/caps.h>
#include <kmi/tcb.h>

/**
 * @file cap.c
 * Sycalls handlers for thread capabilities.
 */

/**
 * Set capabilities.
 *
 * @param t Current tcb.
 * @param tid Thread ID whose capabilities to set.
 * @param caps Mask of capabilities to set.
 * @return \ref OK on success, \ref ERR_INVAL on invalid input.
 */
SYSCALL_DEFINE2(set_cap)(struct tcb *t, sys_arg_t tid, sys_arg_t caps)
{
	if (!is_set(t->caps, CAP_CAPS))
		return_args1(t, ERR_PERM);

	struct tcb *c = get_tcb(tid);
	if (!c)
		return_args1(t, ERR_NF);

	set_caps(c->caps, caps);
	return_args1(t, OK);
}

/**
 * Get capabilities.
 *
 * @param t Current tcb.
 * @param tid Thread ID whose capabilities to get.
 * @return \ref OK, capabilities.
 */
SYSCALL_DEFINE1(get_cap)(struct tcb *t, sys_arg_t tid)
{
	struct tcb *c = get_tcb(tid);
	if (!c)
		return_args1(t, ERR_NF);

	return_args2(t, OK, c->caps);
}

/**
 * Clear capabilities.
 *
 * @param t Current tcb.
 * @param tid Thread ID whose capabilities to clear.
 * @param caps Mask of capabilities to clear.
 * @return ERR_LERM if invalid permissions, ERR_INVAL if \p tid doesn't exist,
 * otherwise OK.
 */
SYSCALL_DEFINE2(clear_cap)(struct tcb *t, sys_arg_t tid, sys_arg_t caps)
{
	if (!is_set(t->caps, CAP_CAPS))
		return_args1(t, ERR_PERM);

	struct tcb *c = get_tcb(tid);
	if (!c)
		return_args1(t, ERR_NF);

	clear_caps(c->caps, caps);
	return_args1(t, OK);
}
