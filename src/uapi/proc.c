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
#include <kmi/assert.h>
#include <kmi/notify.h>
#include <kmi/orphanage.h>
#include <kmi/regions.h>

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
	struct tcb *c = get_cproc(t);
	if (!(has_cap(c->caps, CAP_PROC)))
		return_args1(t, ERR_PERM);

	/* add thread to the effective process */
	struct tcb *e = get_eproc(t);
	assert(e);

	struct tcb *n = create_thread(e);
	if (!n)
		return_args1(t, ERR_OOMEM);

	/* this visit is likely not the cheapest thing in the universe, are
	 * there ways to speed up thread creation? */
	set_thread(n);
	set_ret5(n, n->tid, d0, d1, d2, d3);
	set_return(n, func);

	n->notify_id = t->notify_id;
	return_args1(t, n->tid);
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

	struct tcb *e = get_eproc(t);
	assert(e);

	struct tcb *n = create_proc(e);
	if (!n)
		return_args1(t, ERR_OOMEM);

	/* prepare args for when we eventually swap to the new proc, giving
	 * parent ID as third return value */
	set_args2(n, 0, get_eproc(t)->pid);

	n->notify_id = c->notify_id;
	return_args1(t, n->pid);
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
	if (!is_proc(t))
		return_args1(t, ERR_PERM);

	/* exec is only allowed if we own all our own resources */
	if (t->refcount != 1)
		return_args1(t, ERR_INVAL);

	/* check that bin is at least some kind of valid pointer */
	struct mem_region *b = find_addr_region(&t->uvmem.region, bin);
	if (!b)
		return_args1(t, ERR_ADDR);

	struct mem_region *i = NULL;
	if (interp) {
		/* mark interpreter to be kept */
		i = find_addr_region(&t->uvmem.region, interp);
		if (!i)
			return_args1(t, ERR_ADDR);

	}

	if (prepare_proc(t, bin, interp))
		return_args1(t, ERR_INVAL);

	set_thread(t);
	set_return(t, t->callback);
	set_ret4(t, 0, t->tid, SYS_USER_SPAWNED, t->pid);

	flush_tlb_full();
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

	/** @todo there's currently a kind of silly thing where create_proc()
	 * creates a new thread and calls init_uvmem() on it, and now the elf
	 * loader called from prepare_proc() moves that uvmem aside, calls
	 * init_uvmem() again and finally destroys the old uvmem (assuming
	 * everything went as it was supposed to). Might have to rething these
	 * internal APIs a bit, but for now this ~works~. */
	n->notify_id = c->notify_id;
	if (prepare_proc(n, bin, interp)) {
		destroy_proc(n);
		return_args1(t, ERR_INVAL);
	}

	/** @todo should permissions be transferred? Probably, not but now there
	 * is a slightly annoying asymmetry between fork+exec vs spawn... */

	set_thread(n);
	set_return(n, n->callback);
	set_ret4(n, 0, n->tid, SYS_USER_SPAWNED, n->pid);

	return_args1(t, n->pid);
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
	/* set return value for current thread, important to do first since
	 * use_tcb() switches the register slots, really easy to miss, not great */
	set_args1(t, OK);

	/* switch over to new thread */
	use_tcb(s);

	/* if an irq handler is directly swapping to some other thread,
	 * interpret it as the thread being finished with its critical section */
	enable_irqs();

	if (!is_rpc(s) && orphan(s)) {
		unorphanize(s);
		return;
	}

	/* handle possible queued notification */
	if (s->notify_flags)
		notify(s, 0);

	/* no notifications, so get register state for new thread */
	set_ret(s, 6, get_ret(s));
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
	struct tcb *c = get_proc(t);
	if (!(has_cap(c->caps, CAP_PROC)))
		return_args1(t, ERR_PERM);

	/* init thread is not allowed to exit */
	/** @todo what about other possible threads start at init, are they
	 * allowed to exit? I guess? */
	if (t->tid == 1)
		return_args1(t, ERR_INVAL);

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
 * @param t Current thread.
 * @param tid Thread we want to orphanize. May be ourselves.
 * @return \ref OK on success,
 * \ref ERR_PERM if current process missing \ref CAP_PROC,
 * \ref ERR_INVAL if already an orphant.
 */
SYSCALL_DEFINE1(detach)(struct tcb *t, sys_arg_t tid)
{
	struct tcb *c = get_cproc(t);
	if (!(has_cap(c->caps, CAP_PROC)))
		return_args1(t, ERR_PERM);

	struct tcb *o = get_tcb(tid);
	if (!o || orphan(o))
		return_args1(t, ERR_INVAL);

	orphanize(o);
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
