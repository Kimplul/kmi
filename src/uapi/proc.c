/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file proc.c
 * Process/thread handling syscall implementations.
 */

#include <kmi/elf.h>
#include <kmi/uapi.h>
#include <kmi/proc.h>
#include <kmi/bits.h>
#include <kmi/power.h>
#include <kmi/notify.h>
#include <kmi/orphanage.h>
#include <kmi/mem_regions.h>

#include <arch/irq.h>

/**
 * Create syscall handler.
 *
 * @param t Current tcb.
 * @param func Function to jump to at thread creation.
 * @param d0 Argument 0.
 * @param d1 Argument 1.
 * @param d2 Argument 2.
 * @param d3 Argument 3.
 *
 * @return ERR_OOMEM if thread creation was unsuccessful, otherwise OK and the
 * thread id.
 */
SYSCALL_DEFINE5(create)(struct tcb *t, sys_arg_t func,
                        sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)
{
	struct tcb *c = create_thread(t);
	if (!c)
		return_args1(t, ERR_OOMEM);

	alloc_stack(c);

	set_args5(c, c->tid, d0, d1, d2, d3);
	set_return(c, func);

	c->notify_id = t->notify_id;
	return_args2(t, OK, c->tid);
}

/**
 * Fork syscall handler.
 *
 * \todo Not entirely sure how I should handle forks/execs etc, mostly whether I
 * should allow forks/execs to be called directly or only though the process
 * manager. Probably though the process manager, although that will add in a
 * slight bit of delay.
 *
 * I suppose I could add in a runtime parameter
 * that would allow them to be called directly, and then the process manager
 * would have to periodically ask the kernel about all threads it is aware of
 * via sys_sync. Dunno.
 *
 * @param t Current tcb.
 * @return \ref OK and 0.
 */
SYSCALL_DEFINE0(fork)(struct tcb *t)
{
	struct tcb *c = get_cproc(t);
	if (!(has_cap(c->caps, CAP_PROC)))
		return_args1(t, ERR_PERM);

	struct tcb *n = create_proc(get_eproc(t));
	if (!n)
		return_args1(t, ERR_OOMEM);

	/* prepare args for when we eventually swap to the new proc, giving
	 * parent ID as third return value */
	set_args3(n, OK, 0, get_eproc(t)->pid);

	n->notify_id = c->notify_id;
	return_args2(t, OK, n->pid);
}

/**
 * Exec syscall handler.
 *
 * @param t Current tcb.
 * @param bin Binary to execute.
 * @param interp Optional interpreter binary.
 *
 * @return \see prepare_proc().
 */
SYSCALL_DEFINE2(exec)(struct tcb *t, sys_arg_t bin, sys_arg_t interp)
{
	/** @todo probably make sure thread is root thread of process? */
	/* mark binary to be kept */
	struct mem_region *b = find_used_region(&t->sp_r, bin);
	if (!b)
		return_args1(t, ERR_INVAL);

	set_bit(b->flags, MR_KEEP);

	struct mem_region *i = 0;
	if (interp) {
		/* mark interpreter to be kept */
		i = find_used_region(&t->sp_r, interp);
		if (!i)
			return_args1(t, ERR_INVAL);

		set_bit(i->flags, MR_KEEP);
	}

	/* free everything except regions to be kept */
	clear_uvmem(t);

	/* restore to normal */
	clear_bit(b->flags, MR_KEEP);
	if (interp)
		clear_bit(b->flags, MR_KEEP);

	return_args1(t, prepare_proc(t, bin, interp));
}

/**
 * Spawn syscall handler.
 *
 * @param t Current tcb.
 * @param bin Binary to execute.
 * @param interp Optional interpreter binary.
 *
 * @return \see prepare_proc() and process id of the new process.
 */
SYSCALL_DEFINE2(spawn)(struct tcb *t, sys_arg_t bin, sys_arg_t interp)
{
	struct tcb *c = get_proc(t);
	if (!(has_cap(c->caps, CAP_PROC)))
		return_args1(t, ERR_PERM);

	struct tcb *n = create_proc(NULL);
	if (!n)
		return_args1(t, ERR_OOMEM);

	n->notify_id = c->notify_id;
	return_args2(t, prepare_proc(n, bin, interp), n->pid);
}

/**
 * Kill syscall handler.
 *
 * @param t Current tcb.
 * @param pid Process to kill.
 * \todo Implement.
 *
 * @return ERR_PERM if not capable to kill, otherwise OK.
 */
SYSCALL_DEFINE1(kill)(struct tcb *t, sys_arg_t pid)
{
	struct tcb *c = get_cproc(t);
	if (!(has_cap(c->caps, CAP_PROC)))
		return_args1(t, ERR_PERM);

	struct tcb *r = get_tcb(pid);
	if (is_proc(r))
		return_args1(t, ERR_INVAL);

	destroy_proc(r);
	return_args1(t, OK);
}

/**
 * Actual worker of swapping between threads.
 * Assumes that both \p t and \p s exist and that \p s isn't a zombie or
 * currently running.
 *
 * @param t Current tcb.
 * @param s Thread to swap to.
 */
static void swap(struct tcb *t, struct tcb *s)
{
	/* switch over to new thread */
	use_tcb(s);

	/* if an irq handler is directly swapping to some other thread,
	 * interpret it as the thread being finished with its critical section */
	enable_irqs();

	if (!is_rpc(s) && orphan(s)) {
		orphanize(s);
		return;
	}

	/* set return value for current thread */
	set_args1(t, OK);

	/* handle possible queued notification */
	if (s->notify_flags)
		notify(s, 0);

	/* no notifications, so get register state for new thread */
	return_args(s, get_args(s));
}

/**
 * Syscall handler for exit syscall.
 *
 * @param t Thread that is exiting.
 * @param tid Thread to swap to.
 *
 * @return \ref OK or \ref ERR_INVAL is \p tid is not a thread
 * or \ref ERR_NF if \p tid is a zombie
 * or \ref ERR_EXT if \p tid is currently running.
 */
SYSCALL_DEFINE1(exit)(struct tcb *t, sys_arg_t tid)
{
	if (tid != 0) {
		struct tcb *s = get_tcb(tid);
		if (!s)
			return_args1(t, ERR_INVAL);

		if (zombie(s))
			return_args1(t, ERR_NF);

		if (running(s))
			return_args1(t, ERR_EXT);


		swap(t, s);
	}

	destroy_thread(t);

	if (tid == 0) {
		enable_irqs();
		sleep();
	}
}

/**
 * Syscall handler for orphanizing a thread.
 *
 * @param t Thread that wants to make itself an orphant.
 * @return \ref OK on success,
 * \ref ERR_PERM if current process missing \ref CAP_PROC,
 * \ref ERR_INVAL if already an orphant.
 */
SYSCALL_DEFINE0(detach)(struct tcb *t)
{
	struct tcb *c = get_cproc(t);
	if (!(has_cap(c->caps, CAP_PROC)))
		return_args1(t, ERR_PERM);

	if (orphan(t))
		return_args1(t, ERR_INVAL);

	struct tcb *r = get_tcb(t->rid);
	if (r)
		unreference_proc(r);

	orphanize(t);

	if (!is_rpc(t))
		unorphanize(t);

	return_args1(t, OK);
}

/**
 * Swap syscall handler.
 *
 * \todo Implement.
 * \todo Should swap return the registers of the new thread that would be used
 * for message passing?
 *
 * @param t Current tcb.
 * @param tid Thread ID to swap to.
 *
 * @return \ref OK.
 */
SYSCALL_DEFINE1(swap)(struct tcb *t, sys_arg_t tid){
	struct tcb *c = get_cproc(t);
	if (!(has_cap(c->caps, CAP_PROC)))
		return_args1(t, ERR_PERM);

	struct tcb *s = get_tcb(tid);
	if (!s)
		return_args1(t, ERR_INVAL);

	if (zombie(s))
		return_args1(t, ERR_NF);

	if (running(s))
		return_args1(t, ERR_EXT);

	return swap(t, s);
}
