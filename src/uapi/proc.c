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
#include <kmi/notify.h>
#include <kmi/mem_regions.h>

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
 * @param tid Thread to kill.
 * \todo Implement.
 *
 * @return ERR_PERM if not capable to kill, otherwise OK.
 */
SYSCALL_DEFINE1(kill)(struct tcb *t, sys_arg_t tid)
{
	struct tcb *c = get_cproc(t);
	if (!(has_cap(c->caps, CAP_PROC)))
		return_args1(t, ERR_PERM);

	/** @todo implement */
	/** @todo remember to unregister IRQ handlers */

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

	/* switch over to new thread */
	use_tcb(s);

	/* set return value for current thread */
	set_args1(t, OK);

	/* not running anymore lol */
	if (t->notify_state == NOTIFY_RUNNING)
		t->notify_state = NOTIFY_WAITING;

	/* handle possible queued notification */
	if (s->notify_state == NOTIFY_QUEUED)
		notify(s, 0);

	/* get register state for new thread */
	return_args(s, get_args(s));
}
