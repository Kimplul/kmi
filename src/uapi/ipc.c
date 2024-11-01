/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file ipc.c
 * Interprocess communication syscall implementations.
 */

#include <kmi/orphanage.h>
#include <kmi/debug.h>
#include <kmi/uapi.h>
#include <kmi/bkl.h>
#include <kmi/tcb.h>
#include <kmi/ipi.h>
#include <kmi/irq.h>
#include <kmi/conf.h>

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
enum ipc_flags {
	/** Reuse current rpc stack location. Effectively also means that the
	 * caller gets a response from whoever is called instead of the current
	 * process. */
	IPC_TAIL = (1 << 0),
	/** Don't update the effective ID. */
	IPC_FORWARD = (1 << 1),
	IPC_NOTIFY = (1 << 2),
};

/**
 * Now that we know we're doing an rpc, clone virtual memories and do
 * the slow stuff.
 *
 * @param t Thread to migrate.
 * @param r Process to migrate to.
 * @param flags What kind of IPC we're doing.
 */
static inline void finalize_rpc(struct tcb *t, struct tcb *r, vm_t s)
{
	clone_uvmem(r->proc.vmem, t->rpc.vmem);
	flush_tlb_full();

	set_return(t, r->callback);
	reference_thread(r);
	t->pid = r->rid;

	set_stack(t, s);
}

/**
 * Optimistically assume we're going to take the rpc and do some preparations
 * for it. Most notably, write the arguments as early as possible to
 * free up registers for the compiler to play with.
 *
 * @param t Thread to migrate.
 * @param a RPC arguments.
 * @param flags Kind of IPC we're doing.
 * @return RPC stack difference that should be passed to finalize_rpc().
 */
static inline vm_t enter_rpc(struct tcb *t, struct sys_ret a,
                             enum ipc_flags flags)
{
	/* reuse current rpc stack location if we're being kicked */
	vm_t rpc_stack = (is_set(flags, IPC_TAIL) && is_rpc(t))
	                        ? t->rpc_stack
	                        : rpc_position(t);

	/* rpc_stack points to the top of the region */
	struct call_ctx *ctx = (struct call_ctx *)(rpc_stack) - 1;
	t->regs = (vm_t)ctx;

	/* try to get rid of args as fast as possible to free up registers for
	 * later use */
	set_ret(t, 6, a);

	if (!is_set(flags, IPC_TAIL)) {
		ctx->rpc_stack = t->rpc_stack;
		t->rpc_stack = rpc_stack;
		ctx->exec = t->exec;
		ctx->pid = t->pid;
		ctx->eid = t->eid;
		ctx->notify = flags & IPC_NOTIFY;
		new_rpc(t);
	}
	else {
		/* if we're doing a tail call, we don't need to update any of
		 * the above things */
		reuse_rpc(t);
	}

	return rpc_stack - BASE_PAGE_SIZE;
}

/**
 * Check that there's enough stack left for an rpc invocation.
 *
 * @param t Thread whose migration to check.
 * @return \c true if there's enough stack left to safely do migration,
 * \c false otherwise.
 */
static inline bool __enough_rpc_stack(struct tcb *t)
{
	/* get top of call stack */
	vm_t top = rpc_position(t);

	/* if we can still fit an rpc stack into the call stack, we can safely
	 * do the migration. */
	return top - BASE_PAGE_SIZE - rpc_stack_size() >= RPC_STACK_BASE;
}

/**
 * Actually run notification handler, no ifs or buts.
 * Always enables irqs.
 *
 * @param t Current thread.
 * @param r Process where notification handler is.
 *
 * \p t and \p r may be the same thread.
 */
static __noreturn void __run_notify(struct tcb *t, struct tcb *r)
{
	use_tcb(r);

	enum sys_user code = SYS_USER_NOTIFY;
	enum notify_flag flags = 0;

	/* if we're in the root process, we can safely handle signals and
	 * becoming orphaned */
	if (!is_rpc(t))
		set_bits(flags,
		         t->notify_flags & (NOTIFY_SIGNAL | NOTIFY_ORPHANED));

	/* handle critical notifications with special care */
	if (is_set(t->notify_flags, NOTIFY_IRQ | NOTIFY_TIMER)) {
		set_bits(flags, t->notify_flags & (NOTIFY_IRQ | NOTIFY_TIMER));
		disable_irqs();
	}

	/* signal to whoever is receiving us that we're from the kernel
	 * ("pid 0"), and we are notifying the current thread */
	vm_t s = enter_rpc(t,
	                   SYS_RET5(0, t->tid, code, flags, t->eid),
	                   IPC_NOTIFY);

	finalize_rpc(t, r, s);

	clear_bits(t->notify_flags, flags);

	enable_irqs();
	bkl_unlock();
	ret_userspace_fast();
	unreachable();
}

void notify(struct tcb *t, enum notify_flag flags)
{
	set_bits(t->notify_flags, flags);

	if (!t->notify_flags)
		return;

	struct tcb *r = get_tcb(t->notify_id);
	if (!r || r->state || !r->callback) {
		error("notify callback unavailable\n");
		t->notify_flags = 0;
		return;
	}

	/* signals are only run when thread is in root process, whereas
	 * interrupts are always run (if the resources allow it, that its */
	if (!is_set(t->notify_flags, NOTIFY_IRQ | NOTIFY_TIMER) && is_rpc(t))
		return;

	/* we're either in the root process or we have a critical notification,
	 * check that there's enough rpc stack, otherwise wait and try again
	 * later */
	if (unlikely(!__enough_rpc_stack(r)))
		return;

	/* if someone else is running the thread we would like to send a
	 * notification to, do an ipi. Otherwise, either we're currently running
	 * it or the thread is idle, either is fine for __run_notify(). */
	if (running(r) && r != cur_tcb()) {
		send_ipi(r);
		return;
	}

	__run_notify(t, r);
}

/**
 * Jump back to process where rpc came from, assuming such a thing exists.
 * If we're queued for a notification, jump to it instead.

 * Note that leave_rpc() doesn't enable irqs, as it might be used to
 * cancel a halfway started rpc in do_ipc(), in which case we want to
 * return back to the process with irqs in the same state as before.
 *
 * @param t Thread to do return migration on.
 * @param a Arguments to pass along.
 */
static void leave_rpc(struct tcb *t, struct sys_ret a)
{
	vm_t rpc_stack = t->rpc_stack;
	struct call_ctx *ctx = (struct call_ctx *)(rpc_stack) - 1;
	t->regs = (vm_t)ctx->rpc_stack - sizeof(struct call_ctx);

	/* again, get rid of args as fast as possible */
	if (!ctx->notify)
		set_ret(t, 6, a);

	struct tcb *r = get_tcb(ctx->pid);
	while (!r || !is_proc(r) || zombie(r)) {
		/* we unwound back to our root process which is apparently dead,
		 * we're orphaned :( */
		if (rpc_stack_empty(ctx->rpc_stack)) {
			orphanize(t);
			break;
		}

		rpc_stack = ctx->rpc_stack;
		ctx = (struct call_ctx *)(rpc_stack) - 1;
		t->regs = (vm_t)ctx->rpc_stack - sizeof(struct call_ctx);

		r = get_tcb(ctx->pid);
		/* equivalent to return_args1 but without returning so we can
		 * handle other cases in the loop. */
		if (!ctx->notify)
			set_args1(t, ERR_NF);
	}

	if (orphan(t) && !is_rpc(t))
		unorphanize(t);

	set_return(t, ctx->exec);
	/* if we're returning from a failed rpc, this should essentially be a
	 * no-op */
	t->rpc_stack = ctx->rpc_stack;
	destroy_rpc(t);
	flush_tlb_full();

	t->pid = ctx->pid;
	t->eid = ctx->eid;

	/* notification queued, try to run it */
	if (t->notify_flags)
		notify(t, 0);

	if (!ctx->notify) {
		bkl_unlock();
		ret_userspace_partial();
	}
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
 * @param flags Which kind of IPC to perform.
 *
 * Returns \ref ERR_OOMEM if there isn't enough IPC stack left, \ref ERR_INVAL
 * if the the target process doesn't exist, \ref ERR_NOINIT if the target
 * process hasn't defined a callback. Otherwise \ref OK and whatever the target
 * process sends back.
 *
 * Enables irqs if the ipc is succesful, otherwise leaves them in the state they
 * were when entering.
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
                   enum ipc_flags flags)
{
	if (!is_set(flags, IPC_TAIL) && !__enough_rpc_stack(t))
		return_args1(t, ERR_OOMEM);

	id_t id = is_set(flags, IPC_FORWARD) ? t->eid : t->pid;
	vm_t s = enter_rpc(t, SYS_RET6(id, t->tid, d0, d1, d2, d3), flags);

	struct tcb *r = get_tcb(pid);
	if (unlikely(!r || !is_proc(r))) {
		leave_rpc(t, SYS_RET1(ERR_INVAL));
		return;
	}

	if (unlikely(zombie(r))) {
		leave_rpc(t, SYS_RET1(ERR_INVAL));
		return;
	}

	if (unlikely(!r->callback)) {
		leave_rpc(t, SYS_RET1(ERR_NOINIT));
		return;
	}

	if (!is_set(flags, IPC_FORWARD))
		t->eid = t->pid;

	finalize_rpc(t, r, s);
	/* I tested out passing the return values as arguments to
	 * ret_userspace_fast, but apparently that causes enough stack shuffling
	 * to be slower overall. */
	enable_irqs();
	bkl_unlock();
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
 * @return When succesful: OK, thread id of the handler and the arguments as-is.
 */
SYSCALL_DEFINE5(ipc_req)(struct tcb *t, sys_arg_t pid,
                         sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)
{
	do_ipc(t, pid, d0, d1, d2, d3, 0);
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
 * @return When succesful: OK, thread id of the handler and the arguments as-is.
 */
SYSCALL_DEFINE5(ipc_fwd)(struct tcb *t, sys_arg_t pid,
                         sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)
{
	/* I guess it's fine to check our rpc status and choose flags based on
	 * that, though the path is starting to get a bit convoluted with a
	 * bunch of conditionals that I might want to have a look at getting rid
	 * of at some point, to speed things up a little bit. */
	do_ipc(t, pid, d0, d1, d2, d3, is_rpc(t) ? IPC_FORWARD : 0);
}

/**
 * IPC tail call syscall handler.
 *
 * @param t Current tcb.
 * @param pid Process to request RPC to.
 * @param d0 IPC argument 0.
 * @param d1 IPC argument 1.
 * @param d2 IPC argument 2.
 * @param d3 IPC argument 3.
 * @return When succesful: OK, thread ID of the handler and the arguments as-is.
 */
SYSCALL_DEFINE5(ipc_tail)(struct tcb *t, sys_arg_t pid,
                          sys_arg_t d0, sys_arg_t d1, sys_arg_t d2,
                          sys_arg_t d3)
{
	do_ipc(t, pid, d0, d1, d2, d3, is_rpc(t) ? IPC_TAIL : 0);
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
 * @return When succesful: OK, thread id of the handler and the arguments as-is.
 */
SYSCALL_DEFINE5(ipc_kick)(struct tcb *t, sys_arg_t pid,
                          sys_arg_t d0, sys_arg_t d1, sys_arg_t d2,
                          sys_arg_t d3)
{
	do_ipc(t, pid, d0, d1, d2, d3, is_rpc(t) ? IPC_FORWARD | IPC_TAIL : 0);
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

	/* inform requester who answered (pid) in the case of the request being
	 * kicked forward */
	enable_irqs();
	unreference_thread(get_cproc(t));
	leave_rpc(t, SYS_RET6(OK, t->pid, d0, d1, d2, d3));
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
	if (t->tid != tid && !has_cap(t->caps, CAP_NOTIFY))
		return_args1(t, ERR_PERM);

	struct tcb *r = get_tcb(tid);
	if (!r)
		return_args1(t, ERR_INVAL);

	/* set args, if notify swaps us out we pick them up the next time this
	 * thread is scheduled */
	set_args1(t, OK);
	notify(r, NOTIFY_SIGNAL);
}
