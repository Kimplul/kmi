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
 * @return \ref ERR_EXT and \c 0 if process already is a server,
 * \ref OK and \c 0 otherwise.
 */
SYSCALL_DEFINE1(ipc_server)(sys_arg_t callback)
{
	struct tcb *r = cur_tcb();
	if (r->callback) /* server can't be reinitialized */
		return (struct sys_ret){ ERR_EXT, 0 };

	r->callback = callback;
	return (struct sys_ret){ OK, 0 };
}

/**
 * IPC request syscall handler.
 *
 * @param pid Process to request RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @return \c d0 and \c d1.
 */
SYSCALL_DEFINE3(ipc_req)(sys_arg_t pid, sys_arg_t d0, sys_arg_t d1)
{
	struct tcb *r = get_tcb(pid);
	/** \todo something like jump_to_callback(t) */
	/* remember difference between ipc_req and ipc_fwd! */
	return (struct sys_ret){ d0, d1 };
}

/**
 * IPC forwarding syscall handler.
 *
 * @param pid Process to rquest RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @return \c d0 and \c d1.
 */
SYSCALL_DEFINE3(ipc_fwd)(sys_arg_t pid, sys_arg_t d0, sys_arg_t d1)
{
	struct tcb *t = get_tcb(pid);
	/* ditto */
	return (struct sys_ret){ d0, d1 };
}

/**
 * IPC response syscall handler.
 *
 * @param d0 IPC return value 0.
 * @param d1 IPC return value 1.
 * @return \c d0 and \c d1.
 */
SYSCALL_DEFINE2(ipc_resp)(sys_arg_t d0, sys_arg_t d1)
{
	struct tcb *r = cur_tcb();
	/* something like return_from_callback(t, r) */
	return (struct sys_ret){ d0, d1 };
}
