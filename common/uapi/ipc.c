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
	return SYS_RET1(OK);
}

/**
 * IPC request syscall handler.
 *
 * @param pid Process to request RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @param d2 IPC argument 2.
 * @param d3 IPC argument 3.
 * @return When succesful: OK, thread id of the caller and the arguments as-is.
 */
SYSCALL_DEFINE5(ipc_req)(sys_arg_t pid,
                         sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)
{
	struct tcb *t = cur_tcb();
	struct tcb *r = get_tcb(pid);
	/** \todo something like jump_to_callback(t) */
	/* remember difference between ipc_req and ipc_fwd! */
	return SYS_RET6(OK, t->tid, d0, d1, d2, d3);
}

/**
 * IPC forwarding syscall handler.
 *
 * @param pid Process to request RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @param d2 IPC argument 2.
 * @param d3 IPC argument 3.
 * @return
 */
SYSCALL_DEFINE5(ipc_fwd)(sys_arg_t pid,
                         sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)
{
	struct tcb *t = cur_tcb();
	struct tcb *r = get_tcb(pid);
	/* ditto */
	return SYS_RET6(OK, t->tid, d0, d1, d2, d3);
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
	return SYS_RET6(OK, t->tid, d0, d1, d2, d3);
}

/**
 * Notify syscall handler.
 *
 * \todo Implement.
 *
 * @param tid Thread ID to notify.
 * @param swap Whether to swap immediately if possible.
 * @return \ref OK and 0.
 */
SYSCALL_DEFINE2(ipc_notify)(sys_arg_t tid, sys_arg_t swap){
	/** \todo masquerade as kernel call, set from to 0 and set us as
	 * notify type, no arguments as that would require too much state
	 * handling for my liking. Instead, a server and a client have to agree
	 * on some rpc API, and ipc_notify is just used to asynchronously inform
	 * the client that it should check the status of its async operations.
	 * Arguably slower than directly telling the client which operation was
	 * finished, but this would require the kernel to keep track of a notify
	 * stack. While not impossible, probably too complex. */

	/* Something like
	 *
	 * struct tcb *t = get_tcb(tid);
	 * if (t->notify_state == NOTIFY_QUEUED)
	 *	return;
	 *
	 * if (t->notify_state == NOTIFY_RUNNING) {
	 *	t->notify = NOTIFY_QUEUED;
	 *	return;
	 * }
	 *
	 * t->notify_state = NOTIFY_QUEUED;
	 * if (swap)
	 *	do_swap(); // clears t->notify when swapped to
	 *		   // if already running in base state, interrupt,
	 *		   otherwise wait for return from rpc. If not running,
	 *		   just queue the interrupt.
	 */
	return SYS_RET1(OK);
}
