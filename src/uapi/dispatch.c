/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file dispatch.c
 * Syscall dispatch.
 */

#include <kmi/canary.h>
#include <kmi/debug.h>
#include <kmi/uapi.h>

/* not sure why doxygen requires these two definitions to state their return
 * values, when they don't actually return anything but eh */

/**
 * Noop syscall handler.
 *
 * @param t Current tcb.
 *
 * @return \ref OK and \c 0.
 */
SYSCALL_DEFINE0(noop)(struct tcb *t)
{
	info("sys_noop\n");
	set_args1(t, OK);
}

/**
 * Putch syscall handler.
 *
 * @param t Current tcb.
 * @param a Character to put.
 *
 * @return \ref OK and 0.
 */
SYSCALL_DEFINE1(putch)(struct tcb *t, sys_arg_t a)
{
	dbg("%c", (char)a);
	set_args1(t, OK);
}

void handle_syscall(sys_arg_t syscall, sys_arg_t a, sys_arg_t b,
                    sys_arg_t c, sys_arg_t d, sys_arg_t e, struct tcb *t)
{
	adjust_syscall(t);

	switch (syscall) {
	case SYS_NOOP: sys_noop(t, a, b, c, d, e); break;
	case SYS_PUTCH: sys_putch(t, a, b, c, d, e); break;
	case SYS_REQ_MEM: sys_req_mem(t, a, b, c, d, e); break;
	case SYS_REQ_PMEM: sys_req_pmem(t, a, b, c, d, e); break;
	case SYS_REQ_FIXMEM: sys_req_fixmem(t, a, b, c, d, e); break;
	case SYS_REQ_SHAREDMEM: sys_req_sharedmem(t, a, b, c, d, e); break;
	case SYS_REF_SHAREDMEM: sys_ref_sharedmem(t, a, b, c, d, e); break;
	case SYS_FREE_MEM: sys_free_mem(t, a, b, c, d, e); break;
	case SYS_TIMEBASE: sys_timebase(t, a, b, c, d, e); break;
	case SYS_TICKS: sys_ticks(t, a, b, c, d, e); break;
	case SYS_REQ_NOTIFICATION: sys_req_notification(t, a, b, c, d, e);
		break;
	case SYS_REQ_REL_TIMER: sys_req_rel_timer(t, a, b, c, d, e); break;
	case SYS_REQ_ABS_TIMER: sys_req_abs_timer(t, a, b, c, d, e); break;
	case SYS_IPC_REQ: sys_ipc_req(t, a, b, c, d, e); break;
	case SYS_IPC_FWD: sys_ipc_fwd(t, a, b, c, d, e); break;
	case SYS_IPC_KICK: sys_ipc_kick(t, a, b, c, d, e); break;
	case SYS_IPC_RESP: sys_ipc_resp(t, a, b, c, d, e); break;
	case SYS_IPC_GHOST: sys_ipc_ghost(t, a, b, c, d, e); break;
	case SYS_NOTIFY: sys_notify(t, a, b, c, d, e); break;
	case SYS_CREATE: sys_create(t, a, b, c, d, e); break;
	case SYS_FORK: sys_fork(t, a, b, c, d, e); break;
	case SYS_EXEC: sys_exec(t, a, b, c, d, e); break;
	case SYS_SPAWN: sys_spawn(t, a, b, c, d, e); break;
	case SYS_KILL: sys_kill(t, a, b, c, d, e); break;
	case SYS_SWAP: sys_swap(t, a, b, c, d, e); break;
	case SYS_CONF_SET: sys_conf_set(t, a, b, c, d, e); break;
	case SYS_CONF_GET: sys_conf_get(t, a, b, c, d, e); break;
	case SYS_SET_CAP: sys_set_cap(t, a, b, c, d, e); break;
	case SYS_GET_CAP: sys_get_cap(t, a, b, c, d, e); break;
	case SYS_CLEAR_CAP: sys_clear_cap(t, a, b, c, d, e); break;
	case SYS_POWEROFF: sys_poweroff(t, a, b, c, d, e); break;
	case SYS_SLEEP: sys_sleep(t, a, b, c, d, e); break;
	case SYS_IRQ_REQ: sys_irq_req(t, a, b, c, d, e); break;
	case SYS_DETACH: sys_detach(t, a, b, c, d, e); break;
	case SYS_EXIT: sys_exit(t, a, b, c, d, e); break;
	default:
		error("Syscall %zu outside allowed range [0 - %i]\n", syscall,
		      SYS_NUM - 1);
		set_args1(t, ERR_INVAL);
	};

	if (check_canary(t)) {
		bug("Syscall %zu overwrote stack canary\n", syscall);
	}
}
