#include <apos/uapi.h>

SYSCALL_DEFINE0(create)()
{
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
SYSCALL_DEFINE0(fork)()
{
	return (struct sys_ret){ OK, 0 };
}

SYSCALL_DEFINE3(exec)(sys_arg_t bin, sys_arg_t argc, sys_arg_t argv)
{
	/* TODO: execute new process, probably with more sensible argc passing */
	return (struct sys_ret){ OK, 0 };
}

SYSCALL_DEFINE2(signal)(sys_arg_t tid, sys_arg_t signal)
{
	/* TODO: signals? */
	return (struct sys_ret){ OK, 0 };
}

SYSCALL_DEFINE1(switch)(sys_arg_t tid)
{
	/* TODO: switch to process */
	/* TODO: should switch return the registers of the new thread that would
	 * be used for message passing? */
	return (struct sys_ret){ OK, 0 };
}
