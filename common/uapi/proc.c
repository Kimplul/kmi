/**
 * @file proc.c
 * Process/thread handling syscall implementations.
 */

#include <apos/elf.h>
#include <apos/uapi.h>
#include <apos/proc.h>
#include <apos/bits.h>
#include <apos/mem_regions.h>

SYSCALL_DEFINE0(create)(){
	return (struct sys_ret){ OK, 0 };
}

/* Not entirely sure how I should handle forks/execs etc, mostly whether I
 * should allow forks/execs to be called directly or only though the process
 * manager. Probably though the process manager, although that will add in a
 * slight bit of delay.
 *
 * I suppose I could add in a runtime parameter
 * that would allow them to be called directly, and then the process manager
 * would have to periodically ask the kernel about all threads it is aware of
 * via sys_sync. Dunno.
 */
SYSCALL_DEFINE0(fork)(){
	return (struct sys_ret){ OK, 0 };
}

SYSCALL_DEFINE2(exec)(sys_arg_t bin, sys_arg_t interp){
	/* \todo: execute new process, probably with more sensible argc passing */
	struct tcb *r = cur_tcb();

	/* mark binary to be kept */
	struct mem_region *b = find_used_region(&r->sp_r, bin);
	if (!b)
		return (struct sys_ret){ERR_INVAL, 0};
	set_bit(b->flags, MR_KEEP);

	struct mem_region *i = 0;
	if (interp) {
		/* mark interpreter to be kept */
		i = find_used_region(&r->sp_r, interp);
		if (!i)
			return (struct sys_ret){ERR_INVAL, 1};
		set_bit(i->flags, MR_KEEP);
	}

	/* free everything except regions to be kept */
	clear_uvmem(r, false);

	/* restore to normal */
	clear_bit(b->flags, MR_KEEP);
	if (interp)
		clear_bit(b->flags, MR_KEEP);

	return (struct sys_ret){ prepare_proc(r, bin, interp), 0 };
}

SYSCALL_DEFINE2(signal)(sys_arg_t tid, sys_arg_t signal){
	/* \todo: signals? */
	return (struct sys_ret){ OK, 0 };
}

SYSCALL_DEFINE1(swap)(sys_arg_t tid){
	/* \todo: switch to process */
	/* \todo: should switch return the registers of the new thread that would
	 * be used for message passing? */
	return (struct sys_ret){ OK, 0 };
}
