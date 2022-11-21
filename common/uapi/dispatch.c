/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file dispatch.c
 * Syscall dispatch.
 */

#include <apos/canary.h>
#include <apos/debug.h>
#include <apos/uapi.h>

#include <arch/proc.h>

/** Syscall number to syscall handler conversion. */
static const sys_t syscall_table[] = {
	/* noop */
	[SYS_NOOP] = sys_noop,

	/* debugging */
	[SYS_PUTCH] = sys_putch,

	/* mem */
	[SYS_REQ_MEM] = sys_req_mem,
	[SYS_REQ_PMEM] = sys_req_pmem,
	[SYS_REQ_FIXMEM] = sys_req_fixmem,
	[SYS_FREE_MEM] = sys_free_mem,

	/* timers */
	[SYS_TIMEBASE] = sys_timebase,
	[SYS_TICKS] = sys_ticks,
	[SYS_REQ_REL_TIMER] = sys_req_rel_timer,
	[SYS_REQ_ABS_TIMER] = sys_req_abs_timer,
	[SYS_FREE_TIMER] = sys_free_timer,

	/* ipc */
	[SYS_IPC_SERVER] = sys_ipc_server,
	[SYS_IPC_REQ] = sys_ipc_req,
	[SYS_IPC_FWD] = sys_ipc_fwd,
	[SYS_IPC_RESP] = sys_ipc_resp,
	[SYS_IPC_NOTIFY] = sys_ipc_notify,

	/* proc */
	[SYS_CREATE] = sys_create,
	[SYS_FORK] = sys_fork,
	[SYS_EXEC] = sys_exec,
	[SYS_SPAWN] = sys_spawn,
	[SYS_SWAP] = sys_swap,

	/* conf */
	[SYS_CONF_SET] = sys_conf_set,
	[SYS_CONF_GET] = sys_conf_get,

	[SYS_SET_CAP] = sys_set_cap,
	[SYS_GET_CAP] = sys_get_cap,
	[SYS_CLEAR_CAP] = sys_clear_cap,

	[SYS_POWEROFF] = sys_poweroff,
};

/**
 * Noop syscall handler.
 *
 * @return \ref OK and \c 0.
 */
SYSCALL_DEFINE0(noop)(struct tcb *t)
{
	UNUSED(t);
	info("sys_noop\n");
	return SYS_RET1(OK);
}

/**
 * Putch syscall handler.
 *
 * @param a Character to put.
 * @return \ref OK and 0.
 */
SYSCALL_DEFINE1(putch)(struct tcb *t, sys_arg_t a)
{
	UNUSED(t);

	const char c[2] = {a, 0};
	MAYBE_UNUSED(c);
	dbg((const char *)&c);
	return SYS_RET1(OK);
}

struct sys_ret handle_syscall(struct tcb *t,
                              sys_arg_t syscall, sys_arg_t a, sys_arg_t b,
                              sys_arg_t c, sys_arg_t d, sys_arg_t e)
{
	adjust_syscall(t);

	struct sys_ret r;
	switch (syscall) {
	case SYS_NOOP: r = sys_noop(t, a, b, c, d, e); break;
	case SYS_PUTCH: r = sys_putch(t, a, b, c, d, e); break;
	case SYS_REQ_MEM: r = sys_req_mem(t, a, b, c, d, e); break;
	case SYS_REQ_PMEM: r = sys_req_pmem(t, a, b, c, d, e); break;
	case SYS_REQ_FIXMEM: r = sys_req_fixmem(t, a, b, c, d, e); break;
	case SYS_FREE_MEM: r = sys_free_mem(t, a, b, c, d, e); break;
	case SYS_TIMEBASE: r = sys_timebase(t, a, b, c, d, e); break;
	case SYS_TICKS: r = sys_ticks(t, a, b, c, d, e); break;
	case SYS_REQ_REL_TIMER: r = sys_req_rel_timer(t, a, b, c, d, e); break;
	case SYS_REQ_ABS_TIMER: r = sys_req_abs_timer(t, a, b, c, d, e); break;
	case SYS_IPC_SERVER: r = sys_ipc_server(t, a, b, c, d, e); break;
	case SYS_IPC_REQ: r = sys_ipc_req(t, a, b, c, d, e); break;
	case SYS_IPC_FWD: r = sys_ipc_fwd(t, a, b, c, d, e); break;
	case SYS_IPC_RESP: r = sys_ipc_resp(t, a, b, c, d, e); break;
	case SYS_IPC_NOTIFY: r = sys_ipc_notify(t, a, b, c, d, e); break;
	case SYS_CREATE: r = sys_create(t, a, b, c, d, e); break;
	case SYS_FORK: r = sys_fork(t, a, b, c, d, e); break;
	case SYS_EXEC: r = sys_exec(t, a, b, c, d, e); break;
	case SYS_SPAWN: r = sys_spawn(t, a, b, c, d, e); break;
	case SYS_KILL: r = sys_kill(t, a, b, c, d, e); break;
	case SYS_SWAP: r = sys_swap(t, a, b, c, d, e); break;
	case SYS_CONF_SET: r = sys_conf_set(t, a, b, c, d, e); break;
	case SYS_CONF_GET: r = sys_conf_get(t, a, b, c, d, e); break;
	case SYS_SET_CAP: r = sys_set_cap(t, a, b, c, d, e); break;
	case SYS_GET_CAP: r = sys_get_cap(t, a, b, c, d, e); break;
	case SYS_CLEAR_CAP: r = sys_clear_cap(t, a, b, c, d, e); break;
	case SYS_POWEROFF: r = sys_poweroff(t, a, b, c, d, e); break;
	default:
		error("Syscall %zu outside allowed range [0 - %i]\n", syscall,
		      SYS_NUM - 1);
		r = SYS_RET1(ERR_INVAL);
	};

	if (check_canary(t)) {
		bug("Syscall %zu overwrote stack canary\n", syscall);
	}

	return r;
}
