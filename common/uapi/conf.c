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
	return SYS_RET1(OK);
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
	UNUSED(param);
	UNUSED(val);
	/* no parameters supported atm */

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
	switch (type) {
	case SHUTDOWN:
	case COLD_REBOOT:
	case WARM_REBOOT:
		return SYS_RET2(OK, poweroff(type));
	};

	return SYS_RET1(ERR_INVAL);
}
