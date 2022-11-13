/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file ipc.c
 * Interprocess communication syscall implementations.
 */

#include <apos/uapi.h>
#include <apos/tcb.h>
#include <apos/ipi.h>

/**
 * IPC server notification syscall handler.
 *
 * @param callback Address of server callback.
 * @return \ref OK and \c 0.
 */
SYSCALL_DEFINE1(ipc_server)(sys_arg_t callback)
{
	cur_proc()->callback = callback;
	return SYS_RET1(OK);
}

/**
 * Actual IPC syscall handler.
 *
 * @param pid Process to request RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @param d2 IPC argument 2.
 * @param d3 IPC argument 3.
 * @param fwd Whether to forward.
 * @return
 */
static struct sys_ret do_ipc(sys_arg_t pid,
                             sys_arg_t d0,
                             sys_arg_t d1,
                             sys_arg_t d2,
                             sys_arg_t d3,
                             bool fwd)
{
	struct tcb *t = cur_tcb();
	if (!enough_rpc_stack(t))
		return SYS_RET1(ERR_OOMEM);

	struct tcb *r = get_tcb(pid);
	if (!r)
		return SYS_RET1(ERR_INVAL);

	r = get_rproc(r);

	if (!r->callback)
		return SYS_RET1(ERR_NOINIT);

	clone_uvmem(r->proc.vmem, t->rpc.vmem);
	use_vmem(t->rpc.vmem);
	save_context(t);

	set_return(t, r->callback);
	attach_rpc(r, t);

	if (!fwd)
		t->eid = t->pid;

	t->pid = r->rid;

	return SYS_RET6(OK, t->eid, d0, d1, d2, d3);
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
	return do_ipc(pid, d0, d1, d2, d3, false);
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
	return do_ipc(pid, d0, d1, d2, d3, true);
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
	struct tcb *r = cur_proc();
	load_context(t);
	detach_rpc(r, t);

	if (is_rpc(t))
		use_vmem(t->rpc.vmem);
	else
		use_vmem(t->proc.vmem);

	return SYS_RET6(OK, t->tid, d0, d1, d2, d3);
}

/**
 * Notify syscall handler.
 *
 * \todo Implement.
 *
 * @param tid Thread ID to notify.
 * @return \ref OK and 0.
 */
SYSCALL_DEFINE1(ipc_notify)(sys_arg_t tid){
	struct tcb *t = get_tcb(tid);
	if (t->notify_state == NOTIFY_QUEUED)
		return SYS_RET1(OK);

	if (t->notify_state == NOTIFY_RUNNING) {
		t->notify_state = NOTIFY_QUEUED;
		return SYS_RET1(OK);
	}

	t->notify_state = NOTIFY_QUEUED;
	if (running(t))
		send_ipi(t);

	return SYS_RET1(OK);
}
