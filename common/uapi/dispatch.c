/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file dispatch.c
 * Syscall dispatch.
 */

#include <apos/debug.h>
#include <apos/uapi.h>

static const sys_t syscall_table[] = {
	/* noop */
	[SYS_NOOP] = sys_noop,
	/* mem */
	[SYS_REQ_MEM] = sys_req_mem,
	[SYS_REQ_PMEM] = sys_req_pmem,
	[SYS_REQ_FIXMEM] = sys_req_fixmem,
	[SYS_FREE_MEM] = sys_free_mem,

	/* timers */
	[SYS_TIMEBASE] = sys_timebase,
	[SYS_REQ_REL_TIMER] = sys_req_rel_timer,
	[SYS_REQ_ABS_TIMER] = sys_req_abs_timer,
	[SYS_FREE_TIMER] = sys_free_timer,

	/* ipc */
	[SYS_IPC_SERVER] = sys_ipc_server,
	[SYS_IPC_REQ] = sys_ipc_req,
	[SYS_IPC_FWD] = sys_ipc_fwd,
	[SYS_IPC_RESP] = sys_ipc_resp,

	/* proc */
	[SYS_CREATE] = sys_create,
	[SYS_FORK] = sys_fork,
	[SYS_EXEC] = sys_exec,
	[SYS_SIGNAL] = sys_signal,
	[SYS_SWAP] = sys_swap,

	/* conf */
	[SYS_CONF_SET] = sys_conf_set,
	[SYS_CONF_GET] = sys_conf_get,
	[SYS_POWEROFF] = sys_poweroff,
};

SYSCALL_DEFINE0(noop)(){
	info("sys_noop\n");
	return (struct sys_ret){ OK, 0 };
}

struct sys_ret syscall_dispatch(sys_arg_t syscall, sys_arg_t a, sys_arg_t b,
                                sys_arg_t c, sys_arg_t d)
{
	/** \todo Add check that syscall is not larger than table */
	sys_t call = syscall_table[syscall];

	if (!call)
		return (struct sys_ret){ ERR_INVAL, 0 };

	return call(a, b, c, d);
}
