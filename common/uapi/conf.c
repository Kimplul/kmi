/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file conf.c
 * Runtime configuration sycall implementations.
 *
 * At the moment there are not runtime configuration parameters.
 */

#include <apos/power.h>
#include <apos/sizes.h>
#include <apos/uapi.h>

/** \todo stack size should really be set on a per-thread basis, and are the
 * conf*-syscalls even necessary? */
size_t __thread_stack_size = SZ_2M;
size_t __call_stack_size = SZ_2M;
size_t __rpc_stack_size = SZ_512K;

/** IDs for configuration parameters. */
/** @todo should probably be moved somewhere so it can be shared with userspace */
enum conf_param {
	CONF_THREAD_STACK = 0,
	CONF_CALL_STACK,
	CONF_RPC_STACK,
};

/**
 * Configuration parameter read syscall handler.
 *
 * \todo Implement parameters.
 *
 * @param param Parameter to read.
 * @return \ref OK and parameter value.
 */
SYSCALL_DEFINE1(conf_get)(sys_arg_t param)
{
	struct tcb *t = cur_tcb();
	if (!has_cap(t->caps, CAP_CONF))
		return SYS_RET1(ERR_PERM);

	long val = 0;
	switch (param) {
	case CONF_THREAD_STACK:
		val = __thread_stack_size;
		break;

	case CONF_CALL_STACK:
		val = __call_stack_size;
		break;

	case CONF_RPC_STACK:
		val = __rpc_stack_size;
		break;

	default:
		return SYS_RET1(ERR_NF);
	}

	return SYS_RET2(OK, val);
}

/**
 * Configuration parameter write syscall handler.
 *
 * \todo Implement parameters.
 *
 * @param param Parameter to write.
 * @param val Value to set \c param to.
 * @return \ref OK and \c 0.
 */
SYSCALL_DEFINE2(conf_set)(sys_arg_t param, sys_arg_t val)
{
	struct tcb *t = cur_tcb();
	if (!has_cap(t->caps, CAP_CONF))
		return SYS_RET1(ERR_PERM);

	size_t size = 0;
	switch (param) {
	case CONF_THREAD_STACK:
		__thread_stack_size = align_up(val, BASE_PAGE_SIZE);
		break;

	case CONF_CALL_STACK:
		size = align_up(val, 4 * BASE_PAGE_SIZE);
		if (size < __rpc_stack_size * 4)
			return SYS_RET1(ERR_MISC);

		__call_stack_size = size;
		break;

	case CONF_RPC_STACK:
		size = align_up(val, BASE_PAGE_SIZE);
		if (size > __call_stack_size / 4)
			return SYS_RET1(ERR_MISC);

		__rpc_stack_size = size;
		break;
	}

	return SYS_RET1(OK);
}

/**
 * Poweroff syscall handler.
 *
 * @param type Type of poweroff.
 * @return \ref ERR_INVAL and \c 0 if incorrect poweroff \c type give, otherwise
 * does not return.
 */
SYSCALL_DEFINE1(poweroff)(sys_arg_t type)
{
	struct tcb *t = cur_tcb();
	if (!(has_cap(t->caps, CAP_POWER)))
		return SYS_RET1(ERR_PERM);

	switch (type) {
	case SHUTDOWN:
	case COLD_REBOOT:
	case WARM_REBOOT:
		return SYS_RET2(OK, poweroff(type));
	};

	return SYS_RET1(ERR_INVAL);
}
