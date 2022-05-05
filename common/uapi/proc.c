#include <apos/uapi.h>

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
SYSCALL_DEFINE1(fork)(vm_t pid)
{
	/* fork might not actually even need pid...? */
	/* TODO: create new thread in the same process family */
	return 0;
}

SYSCALL_DEFINE4(exec)(vm_t pid, vm_t bin, vm_t argc, vm_t argv)
{
	/* TODO: execute new process */
	return 0;
}

SYSCALL_DEFINE2(signal)(vm_t pid, vm_t signal)
{
	/* TODO: signals? */
	return 0;
}

SYSCALL_DEFINE1(switch)(vm_t pid)
{
	/* TODO: switch to process */
	return 0;
}
