/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file ipc.c
 * Interprocess communication syscall implementations.
 */

#include <apos/uapi.h>
#include <apos/tcb.h>

/**
 * IPC server notification syscall handler.
 *
 * @param callback Address of server callback.
 * @return \ref OK and \c 0.
 */
SYSCALL_DEFINE1(ipc_server)(sys_arg_t callback)
{
	cur_tcb()->callback = callback;
	return (struct sys_ret){ OK, 0, 0, 0, 0, 0 };
}

/**
 * IPC request syscall handler.
 *
 * @param pid Process to request RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @param d2 IPC argument 2.
 * @param d3 IPC argument 3.
 * @return \c d0 and \c d1.
 */
SYSCALL_DEFINE5(ipc_req)(sys_arg_t pid,
                         sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)
{
	struct tcb *t = cur_tcb();
	struct tcb *r = get_tcb(pid);
	/** \todo something like jump_to_callback(t) */
	/* remember difference between ipc_req and ipc_fwd! */
	return (struct sys_ret){ OK, t->tid, d0, d1, d2, d3 };
}

/**
 * IPC forwarding syscall handler.
 *
 * @param pid Process to request RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @param d2 IPC argument 2.
 * @param d3 IPC argument 3.
 * @return \p d0 and \p d1.
 */
SYSCALL_DEFINE5(ipc_fwd)(sys_arg_t pid,
                         sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)
{
	struct tcb *t = cur_tcb();
	struct tcb *r = get_tcb(pid);
	/* ditto */
	return (struct sys_ret){ OK, t->tid, d0, d1, d2, d3 };
}

/**
 * IPC response syscall handler.
 *
 * @param d0 IPC return value 0.
 * @param d1 IPC return value 1.
 * @param d2 IPC return value 2.
 * @param d3 IPC return value 3.
 * @return \c d0 and \c d1.
 */
SYSCALL_DEFINE4(ipc_resp)(sys_arg_t d0, sys_arg_t d1, sys_arg_t d2,
                          sys_arg_t d3)
{
	struct tcb *t = cur_tcb();
	/* something like return_from_callback(t, r) */
	return (struct sys_ret){ OK, t->tid, d0, d1, d2, d3 };
}

/**
 * Notify syscall handler.
 *
 * \todo Implement.
 *
 * @param tid Thread ID to notify.
 * @param swap Whether to swap immediately if possible.
 * @param a0 Argument 0.
 * @param a1 Argument 1.
 * @return \ref OK and 0.
 */
SYSCALL_DEFINE4(ipc_notify)(sys_arg_t tid, sys_arg_t swap,
                            sys_arg_t a0, sys_arg_t a1){
	/** \todo masquerade as kernel call, set from to 0 and set us as
	 * notify type, with arguments a0 and a1 as user-configurable data. */
	return (struct sys_ret){ OK, 0, 0 /* type */, 0 /* from */, a0, a1 };
}
