/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file ipc.c
 * Interprocess communication syscall implementations.
 */

#include <kmi/uapi.h>
#include <kmi/tcb.h>
#include <kmi/ipi.h>

/**
 * IPC server notification syscall handler.
 *
 * @param t Current tcb.
 * @param callback Address of server callback.
 * @return \ref OK and \c 0.
 */
SYSCALL_DEFINE1(ipc_server)(struct tcb *t, sys_arg_t callback)
{
	get_cproc(t)->callback = callback;
	return_args(t, SYS_RET1(OK));
}

/**
 * Actual IPC syscall handler.
 *
 * @param t Current tcb.
 * @param pid Process to request RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @param d2 IPC argument 2.
 * @param d3 IPC argument 3.
 * @param fwd Whether to forward.
 *
 * Returns \ref ERR_OOMEM if there isn't enough IPC stack left, \ref ERR_INVAL
 * if the the target process doesn't exist, \ref ERR_NOINIT if the target
 * process hasn't defined a callback. Otherwise \ref OK and whatever the target
 * process sends back.
 */
static void do_ipc(struct tcb *t,
                   sys_arg_t pid,
                   sys_arg_t d0,
                   sys_arg_t d1,
                   sys_arg_t d2,
                   sys_arg_t d3,
                   bool fwd)
{
	if (!enough_rpc_stack(t))
		return_args(t, SYS_RET1(ERR_OOMEM));

	struct tcb *r = get_tcb(pid);
	if (!r)
		return_args(t, SYS_RET1(ERR_INVAL));

	r = get_rproc(r);

	if (!r->callback)
		return_args(t, SYS_RET1(ERR_NOINIT));

	clone_uvmem(r->proc.vmem, t->rpc.vmem);
	enter_rpc(t);

	set_return(t, r->callback);
	attach_rpc(r, t);

	if (!fwd)
		t->eid = t->pid;

	t->pid = r->rid;

	return_args(t, SYS_RET6(OK, t->eid, d0, d1, d2, d3));
}
/**
 * IPC request syscall handler.
 *
 * @param t Current tcb.
 * @param pid Process to request RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @param d2 IPC argument 2.
 * @param d3 IPC argument 3.
 * @return When succesful: OK, thread id of the caller and the arguments as-is.
 */
SYSCALL_DEFINE5(ipc_req)(struct tcb *t, sys_arg_t pid,
                         sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)
{
	do_ipc(t, pid, d0, d1, d2, d3, false);
}

/**
 * IPC forwarding syscall handler.
 *
 * @param t Current tcb.
 * @param pid Process to request RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @param d2 IPC argument 2.
 * @param d3 IPC argument 3.
 * @return
 */
SYSCALL_DEFINE5(ipc_fwd)(struct tcb *t, sys_arg_t pid,
                         sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)
{
	do_ipc(t, pid, d0, d1, d2, d3, true);
}

/**
 * IPC response syscall handler.
 *
 * @param t Current tcb.
 * @param d0 IPC return value 0.
 * @param d1 IPC return value 1.
 * @param d2 IPC return value 2.
 * @param d3 IPC return value 3.
 * @return \c d0 and \c d1.
 */
SYSCALL_DEFINE4(ipc_resp)(struct tcb *t, sys_arg_t d0, sys_arg_t d1,
                          sys_arg_t d2,
                          sys_arg_t d3)
{
	struct tcb *r = get_cproc(t);
	leave_rpc(t);
	detach_rpc(r, t);

	if (is_rpc(t))
		use_vmem(t->rpc.vmem);
	else
		use_vmem(t->proc.vmem);

	return_args(t, SYS_RET6(OK, t->tid, d0, d1, d2, d3));
}

/**
 * Notify syscall handler.
 *
 * \todo Implement.
 *
 * @param t Current tcb.
 * @param tid Thread ID to notify.
 * @return \ref OK and 0.
 */
SYSCALL_DEFINE1(ipc_notify)(struct tcb *t, sys_arg_t tid){
	if (!has_cap(t->caps, CAP_CALL))
		return_args(t, SYS_RET1(ERR_PERM));

	struct tcb *r = get_tcb(tid);
	if (r->notify_state == NOTIFY_QUEUED)
		return_args(t, SYS_RET1(OK));

	if (r->notify_state == NOTIFY_RUNNING) {
		t->notify_state = NOTIFY_QUEUED;
		return_args(t, SYS_RET1(OK));
	}

	r->notify_state = NOTIFY_QUEUED;
	if (running(r))
		send_ipi(r);

	return_args(t, SYS_RET1(OK));
}
