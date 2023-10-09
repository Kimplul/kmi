/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file ipc.c
 * Interprocess communication syscall implementations.
 */

#include <kmi/uapi.h>
#include <kmi/tcb.h>
#include <kmi/ipi.h>
#include <kmi/conf.h>

/** Structure for maintaining the required context data for an rpc call. */
struct call_ctx {
	/** Execution continuation point. */
	vm_t exec;

	/** Register save area. */
	vm_t regs;

	/** Position in rpc stack. */
	vm_t rpc_stack;

	/** Effective process ID. */
	id_t eid;

	/** Current process ID. */
	id_t pid;

	/** Whether this context should be skipped when responding. */
	bool kick;
};

/**
 * Represents difference between where rpc stack was before rpc call and during.
 * Used to figure out which areas should be marked inaccessible.
 */
struct stack_diff {
	/** Current stack start. Keep in mind that stacks grow down. */
	vm_t start;
	/** Difference end, i.e. previous stack start. */
	vm_t end;
};

/** Enumerator for IPC kind. Used by do_ipc(). */
enum ipc_kind {
	IPC_REQ, IPC_FWD, IPC_KICK
};

/**
 * Now that we know we're doing an rpc, clone virtual memories and do
 * the slow stuff.
 *
 * @param t Thread to migrate.
 * @param r Process to migrate to.
 * @param s RPC stack regions to mark inaccessible.
 */
static void finalize_rpc(struct tcb *t, struct tcb *r, vm_t s)
{
	clone_uvmem(r->proc.vmem, t->rpc.vmem);
	set_return(t, r->callback);
	reference_proc(r);
	t->pid = r->rid;

	/* make sure updates are visible when swapping to the new virtual memory */
	mark_rpc_invalid(t, s);
	use_vmem(t->rpc.vmem);
}

/**
 * Optimistically assume we're going to take the rpc and do some preparations
 * for it. Most notably, write the arguments as early as possible to
 * free up registers for the compiler to play with.
 *
 * @param t Thread to migrate.
 * @param a RPC arguments.
 * @param kind Kind of IPC we're doing. Essentially toggles kick boolean.
 * @return RPC stack difference that should be passed to finalize_rpc().
 */
static vm_t enter_rpc(struct tcb *t, struct sys_ret a,
                                   enum ipc_kind kind)
{
	vm_t rpc_stack = rpc_position(t);

	struct call_ctx *ctx = (struct call_ctx *)(rpc_stack) - 1;
	ctx->regs = t->regs;
	t->regs = (vm_t)ctx;

	/* try to get rid of args as fast as possible to free up registers for
	 * later use */
	set_args(t, 6, a);

	ctx->exec = t->exec;
	ctx->pid = t->pid;
	ctx->eid = t->eid;
	ctx->rpc_stack = rpc_stack;

	/* only rpcs can be kicked forward */
	ctx->kick = kind == IPC_KICK && is_rpc(t);

	/** @todo if we run out of rpc_stack space we should just stop, likely
	 * return a status? except it shouldn't happen after we've run
	 * enough_rpc_stack(). */
	vm_t new_stack = rpc_stack - BASE_PAGE_SIZE;

	/** @todo what if each stack is only some number of pages, and if a proc
	 * goes over the limit is is seen as programming error? Possibly user
	 * configurable number as well, might actually use the config subsystem
	 * :D
	 * In such a case it would probably be smarter to mark all pages
	 * inaccessible at first, and then mark the first page accessible. If
	 * the process needs more stack space it'll cause a paging exception,
	 * we'll handle it separately and if the process isn't going over the
	 * limit just give it more.
	 * */
	t->rpc_stack = new_stack;
	set_stack(t, new_stack);
	return new_stack;
}

/**
 * Jump back to process where rpc came from, assuming such a thing exists.
 *
 * @param t Thread to do return migration on.
 * @param a Arguments to pass along.
 */
static void leave_rpc(struct tcb *t, struct sys_ret a)
{
	vm_t rpc_stack = t->rpc_stack + BASE_PAGE_SIZE;
	struct call_ctx *ctx = (struct call_ctx *)(rpc_stack) - 1;
	vm_t top = ctx->rpc_stack;

	/* find first instance of not kicked context */
	while (ctx->kick) {
		rpc_stack = ctx->rpc_stack + BASE_PAGE_SIZE;
		ctx = (struct call_ctx *)(rpc_stack) - 1;
		unreference_proc(get_tcb(ctx->pid));
	}

	t->regs = ctx->regs;
	/* again, get rid of args as fast as possible */
	set_args(t, 6, a);

	set_return(t, ctx->exec);
	/* if we're returning from a failed rpc, this should essentially be a
	 * no-op */
	mark_rpc_valid(t, top);
	t->rpc_stack = ctx->rpc_stack;
	t->pid = ctx->pid;
	t->eid = ctx->eid;

	if (is_rpc(t))
		use_vmem(t->rpc.vmem);
	else
		use_vmem(t->proc.vmem);
}

/**
 * Check that there's enough stack left for an rpc invocation.
 *
 * @param t Thread whose migration to check.
 * @return \c true if there's enough stack left to safely do migration,
 * \c false otherwise.
 */
static bool enough_rpc_stack(struct tcb *t)
{
	/* get top of call stack */
	vm_t top = rpc_position(t);

	/* if we can still fit an rpc stack into the call stack, we can safely
	 * do the migration. */
	return (top - BASE_PAGE_SIZE) >= (t->rpc_stack - __rpc_stack_size);
}

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
	return_args1(t, OK);
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
 * @param kind Which kind of IPC to perform.
 *
 * Returns \ref ERR_OOMEM if there isn't enough IPC stack left, \ref ERR_INVAL
 * if the the target process doesn't exist, \ref ERR_NOINIT if the target
 * process hasn't defined a callback. Otherwise \ref OK and whatever the target
 * process sends back.
 *
 * @todo should all static functions have double underscores? I seem to be
 * inconsistent.
 */
static void do_ipc(struct tcb *t,
                   sys_arg_t pid,
                   sys_arg_t d0,
                   sys_arg_t d1,
                   sys_arg_t d2,
                   sys_arg_t d3,
                   enum ipc_kind kind)
{
	if (unlikely(!enough_rpc_stack(t)))
		return_args1(t, ERR_OOMEM);

	vm_t s = enter_rpc(t, SYS_RET6(OK, t->eid, d0, d1, d2, d3), kind);

	struct tcb *r = get_tcb(pid);
	if (unlikely(!r)) {
		leave_rpc(t, SYS_RET1(ERR_INVAL));
		return;
	}

	r = get_rproc(r);
	if (unlikely(r->dead)) {
		leave_rpc(t, SYS_RET1(ERR_INVAL));
		return;
	}

	if (unlikely(!r->callback)) {
		leave_rpc(t, SYS_RET1(ERR_NOINIT));
		return;
	}

	if (kind != IPC_REQ)
		t->eid = t->pid;

	finalize_rpc(t, r, s);
	/* I tested out passing the return values as arguments to
	 * ret_userspace_fast, but apparently that causes enough stack shuffling
	 * to be slower overall. */
	ret_userspace_fast();
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
	do_ipc(t, pid, d0, d1, d2, d3, IPC_REQ);
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
 * @return When succesful: OK, thread id of the caller and the arguments as-is.
 */
SYSCALL_DEFINE5(ipc_fwd)(struct tcb *t, sys_arg_t pid,
                         sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)
{
	do_ipc(t, pid, d0, d1, d2, d3, IPC_FWD);
}

/**
 * IPC kicking syscall handler.
 *
 * @param t Current tcb.
 * @param pid Process to request RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @param d2 IPC argument 2.
 * @param d3 IPC argument 3.
 * @return When succesful: OK, thread id of the caller and the arguments as-is.
 */
SYSCALL_DEFINE5(ipc_kick)(struct tcb *t, sys_arg_t pid,
                          sys_arg_t d0, sys_arg_t d1, sys_arg_t d2,
                          sys_arg_t d3)
{
	do_ipc(t, pid, d0, d1, d2, d3, IPC_KICK);
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
	/* if we're not in an rpc, the user messed something up. */
	/** @todo choose or come up with more fitting error value. */
	if (unlikely(!is_rpc(t)))
		return_args1(t, ERR_MISC);

	leave_rpc(t, SYS_RET6(OK, t->tid, d0, d1, d2, d3));
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
		return_args1(t, ERR_PERM);

	struct tcb *r = get_tcb(tid);
	if (r->notify_state == NOTIFY_QUEUED)
		return_args1(t, OK);

	if (r->notify_state == NOTIFY_RUNNING) {
		t->notify_state = NOTIFY_QUEUED;
		return_args1(t, OK);
	}

	r->notify_state = NOTIFY_QUEUED;
	if (running(r))
		send_ipi(r);

	return_args1(t, OK);
}
