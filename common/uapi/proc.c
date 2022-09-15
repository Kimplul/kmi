/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file proc.c
 * Process/thread handling syscall implementations.
 */

#include <apos/elf.h>
#include <apos/uapi.h>
#include <apos/proc.h>
#include <apos/bits.h>
#include <apos/mem_regions.h>

/**
 * Create syscall handler.
 *
 * \todo Implement.
 *
 * @param func Function to jump to at thread creation.
 * @param arg Argument to pass to the function.
 *
 * @return \ref OK and 0.
 */
SYSCALL_DEFINE2(create)(sys_arg_t func, sys_arg_t arg){
	return (struct sys_ret){ OK, 0, 0, 0, 0, 0 };
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
 * @return \ref OK and 0.
 */
SYSCALL_DEFINE0(fork)(){
	return (struct sys_ret){ OK, 0, 0, 0, 0, 0 };
}

/**
 * Exec syscall handler.
 *
 * @param bin Binary to execute.
 * @param interp Optional interpreter binary.
 * @return \see prepare_proc() and 0.
 */
SYSCALL_DEFINE2(exec)(sys_arg_t bin, sys_arg_t interp){
	/** \todo execute new process, probably with more sensible argc passing */
	struct tcb *r = cur_tcb();

	/* mark binary to be kept */
	struct mem_region *b = find_used_region(&r->sp_r, bin);
	if (!b)
		return (struct sys_ret){ERR_INVAL, 0, 0, 0, 0, 0};
	set_bit(b->flags, MR_KEEP);

	struct mem_region *i = 0;
	if (interp) {
		/* mark interpreter to be kept */
		i = find_used_region(&r->sp_r, interp);
		if (!i)
			return (struct sys_ret){ERR_INVAL, 1, 0, 0, 0, 0};
		set_bit(i->flags, MR_KEEP);
	}

	/* free everything except regions to be kept */
	clear_uvmem(r);

	/* restore to normal */
	clear_bit(b->flags, MR_KEEP);
	if (interp)
		clear_bit(b->flags, MR_KEEP);

	return (struct sys_ret){ prepare_proc(r, bin, interp), 0, 0, 0, 0, 0 };
}

/**
 * Kill syscall handler.
 *
 * \todo Implement.
 *
 * @return No?
 */
SYSCALL_DEFINE0(kill)()
{
	return (struct sys_ret){ OK, 0, 0, 0, 0, 0};
}

/**
 * Signal syscall handler.
 *
 * \todo Implement.
 *
 * @param tid Thread ID to signal.
 * @param signal Signal to send to \c tid.
 * @param swap Whether to immediately swap to thread.
 * @return \ref OK and 0.
 */
SYSCALL_DEFINE3(signal)(sys_arg_t tid, sys_arg_t signal, sys_arg_t swap){
	return (struct sys_ret){ OK, 0, 0, 0, 0, 0 };
}

/**
 * Swap syscall handler.
 *
 * \todo Implement.
 * \todo Should swap return the registers of the new thread that would be used
 * for message passing?
 *
 * @param tid Thread ID to swap to.
 * @return \ref OK and 0.
 */
SYSCALL_DEFINE1(swap)(sys_arg_t tid){
	/** \todo switch to process */
	/** \todo should switch return the registers of the new thread that would
	 * be used for message passing? */
	return (struct sys_ret){ OK, 0, 0, 0, 0, 0 };
}
