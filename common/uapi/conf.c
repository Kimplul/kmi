/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file conf.c
 * Runtime configuration sycall implementations.
 *
 * At the moment there are not runtime configuration parameters.
 */

#include <kmi/power.h>
#include <kmi/sizes.h>
#include <kmi/uapi.h>
#include <kmi/conf.h>

#include <arch/proc.h>

/** \todo stack size should really be set on a per-thread basis, and are the
 * conf*-syscalls even necessary? */
size_t __thread_stack_size = SZ_2M;
size_t __rpc_stack_size = SZ_512K;

/** IDs for configuration parameters. */
/** @todo should probably be moved somewhere so it can be shared with userspace */
enum conf_param {
	CONF_THREAD_STACK = 0,
	CONF_RPC_STACK,
};

/**
 * Configuration parameter read syscall handler.
 *
 * \todo Implement parameters.
 *
 * @param t Current tcb.
 * @param param Parameter to read.
 * @return \ref OK and parameter value.
 */
SYSCALL_DEFINE1(conf_get)(struct tcb *t, sys_arg_t param)
{
	if (!has_cap(t->caps, CAP_CONF))
		return_args1(t, ERR_PERM);

	long val = 0;
	switch (param) {
	case CONF_THREAD_STACK:
		val = __thread_stack_size;
		break;

	case CONF_RPC_STACK:
		val = __rpc_stack_size;
		break;

	default:
		return_args1(t, ERR_NF);
	}

	return_args2(t, OK, val);
}

/**
 * Configuration parameter write syscall handler.
 *
 * \todo Implement parameters.
 *
 * @param t Current tcb.
 * @param param Parameter to write.
 * @param val Value to set \c param to.
 * @return \ref OK and \c 0.
 */
SYSCALL_DEFINE2(conf_set)(struct tcb *t, sys_arg_t param, sys_arg_t val)
{
	if (!has_cap(t->caps, CAP_CONF))
		return_args1(t, ERR_PERM);

	size_t size = 0;
	switch (param) {
	case CONF_THREAD_STACK:
		__thread_stack_size = align_up(val, BASE_PAGE_SIZE);
		break;

	case CONF_RPC_STACK:
		size = align_up(val, BASE_PAGE_SIZE);
		if (size > max_rpc_size())
			return_args1(t, ERR_MISC);

		__rpc_stack_size = size;
		break;
	}

	return_args1(t, OK);
}

/**
 * Poweroff syscall handler.
 *
 * @param t Current tcb.
 * @param type Type of poweroff.
 * @return \ref ERR_INVAL and \c 0 if incorrect poweroff \c type give, otherwise
 * does not return.
 */
SYSCALL_DEFINE1(poweroff)(struct tcb *t, sys_arg_t type)
{
	if (!(has_cap(t->caps, CAP_POWER)))
		return_args1(t, ERR_PERM);

	switch (type) {
	case SHUTDOWN:
	case COLD_REBOOT:
	case WARM_REBOOT:
		return_args2(t, OK, poweroff(type));
	};

	return_args1(t, ERR_INVAL);
}
