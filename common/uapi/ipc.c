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

/**
 * Mark rpc stack between \p start and \p end inaccessible.
 *
 * @param t Thread whose rpc stack to modify.
 * @param start Start address of rpc stack to mark inaccessible.
 * @param end End address of rpc stack to mark inaccessible.
 */
static void mark_rpc_inaccessible(struct tcb *t, vm_t start, vm_t end)
{
	size_t page_size = BASE_PAGE_SIZE;
	size_t size = end - start;
	size_t pages = size / page_size;
	while (pages--)
		clear_vpage_flags(t->rpc.vmem, start + pages * page_size, VM_U);
}

/**
 * Mark rpc stack between \p start and \p end accessible.
 *
 * @param t Thread whose rpc stack to modify.
 * @param start Start address of rpc stack to mark accessible.
 * @param end End address of rpc stack to mark accessible.
 */
static void mark_rpc_accessible(struct tcb *t, vm_t start, vm_t end)
{
	size_t page_size = BASE_PAGE_SIZE;
	size_t size = end - start;
	size_t pages = size / page_size;
	while (pages--)
		set_vpage_flags(t->rpc.vmem, start + pages * page_size, VM_U);
}

/** Structure for maintaingin the required context data for an rpc call. */
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

/**
 * Now that we know we're doing an rpc, clone virtual memories and do
 * the slow stuff.
 *
 * @param t Thread to migrate.
 * @param r Process to migrate to.
 * @param sd RPC stack regions to mark inaccessible.
 */
static void finalize_rpc(struct tcb *t, struct tcb *r, struct stack_diff sd)
{
	clone_uvmem(r->proc.vmem, t->rpc.vmem);
	set_return(t, r->callback);
	attach_rpc(r, t);
	t->pid = r->rid;

	/* make sure updates are visible when swapping to the new virtual memory */
	mark_rpc_inaccessible(t, sd.start, sd.end);
	use_vmem(t->rpc.vmem);
}

/**
 * Optimistically assume we're going to take the rpc and do some preparations
 * for it. Most notably, write the arguments as early as possible to
 * free up registers for the compiler to play with.
 *
 * @param t Thread to migrate.
 * @param a RPC arguments.
 * @return RPC stack difference that should be passed to finalize_rpc().
 */
static struct stack_diff enter_rpc(struct tcb *t, struct sys_ret a)
{
	vm_t rpc_stack = t->rpc_stack;
	if (is_rpc(t))
		/** @todo what if user uses their own stack? Or is a dick and
		 * sets the stack pointer to RPC_STACK_TOP or something? It'll
		 * likely only cause a fuckup in the process who did the dumb
		 * thing, so maybe just consider it user error? Except by
		 * causing the stack of the next rpc to run out of memory... */
		rpc_stack = align_down(get_stack(t), BASE_PAGE_SIZE);


	struct call_ctx *ctx = (struct call_ctx *)(rpc_stack) - 1;
	ctx->regs = t->regs;
	t->regs = (vm_t)ctx;

	/* try to get rid of args as fast as possible to free up registers for
	 * later use */
	set_args(t, a);

	ctx->exec = t->exec;
	ctx->pid = t->pid;
	ctx->eid = t->eid;
	ctx->rpc_stack = rpc_stack;

	/** @todo if we run out of rpc_stack space we should just stop, likely
	 * return a status? except it shouldn't happen after we've run
	 * enough_rpc_stack(). */
	vm_t new_stack = rpc_stack - BASE_PAGE_SIZE;
	struct stack_diff sd = {new_stack, t->rpc_stack};

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
	return sd;
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
	t->regs = ctx->regs;
	/* again, get rid of args as fast as possible */
	set_args(t, a);

	set_return(t, ctx->exec);
	/* if we're returning from a failed rpc, this should essentially be a
	 * no-op */
	mark_rpc_accessible(t, t->rpc_stack, ctx->rpc_stack);
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
	vm_t top = RPC_STACK_BASE + __call_stack_size;
	vm_t rpc_stack = t->rpc_stack + BASE_PAGE_SIZE;

	return top - rpc_stack >= __call_stack_size / RPC_STACK_RATIO;
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
	if (unlikely(!enough_rpc_stack(t)))
		return_args(t, SYS_RET1(ERR_OOMEM));

	struct stack_diff sd =
		enter_rpc(t, SYS_RET6(OK, t->eid, d0, d1, d2, d3));

	struct tcb *r = get_tcb(pid);
	if (unlikely(!r)) {
		leave_rpc(t, SYS_RET1(ERR_INVAL));
		return;
	}

	r = get_rproc(r);

	if (unlikely(!r->callback)) {
		leave_rpc(t, SYS_RET1(ERR_NOINIT));
		return;
	}

	if (!fwd)
		t->eid = t->pid;

	finalize_rpc(t, r, sd);
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
	leave_rpc(t, SYS_RET6(OK, t->tid, d0, d1, d2, d3));
	detach_rpc(r, t);
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
