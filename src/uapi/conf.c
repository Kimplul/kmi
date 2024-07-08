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
#include <arch/irq.h>

#include <arch/proc.h>

/** \todo stack size should probably be set on a per-thread basis */

/** Current global thread stack size. */
static size_t __thread_stack_size = SZ_2M;

/** Current global RPC stack entry size. */
static size_t __rpc_stack_size = SZ_512K;


size_t thread_stack_size()
{
	return __thread_stack_size;
}

size_t rpc_stack_size()
{
	return __rpc_stack_size;
}

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
	/* anyone can read any current parameter, I don't think they should be
	 * hidden. */
	long val = 0;
	switch (param) {
	case CONF_THREAD_STACK:
		val = __thread_stack_size;
		break;

	case CONF_RPC_STACK:
		val = __rpc_stack_size;
		break;

	case CONF_RAM_USAGE:
		val = query_used();
		break;

	case CONF_RAM_SIZE:
		val = get_ram_size();
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

	default:
		return_args1(t, ERR_INVAL);
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
		return_args1(t, poweroff(type));
	};

	return_args1(t, ERR_INVAL);
}

/**
 * Sleep syscall handler.
 *
 * @param t Current tcb.
 * @return Shouldn't, but \ref ERR_PERM if insufficient permissions,
 * otherwise whatever \ref sleep() returns.
 */
SYSCALL_DEFINE0(sleep)(struct tcb *t)
{
	if (!(has_cap(t->caps, CAP_POWER)))
		return_args1(t, ERR_PERM);

	/* presumably we want to wake up on an interrupt */
	enable_irqs();
	return_args1(t, sleep());
}
