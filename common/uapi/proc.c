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
vm_t sys_fork(vm_t pid, vm_t u0, vm_t u1, vm_t u2)
{
	UNUSED(u0); UNUSED(u1); UNUSED(u2);
	/* TODO: create new thread in the same process family */
	return 0;
}

vm_t sys_exec(vm_t pid, vm_t bin, vm_t argc, vm_t argv)
{
	/* TODO: execute new process */
	return 0;
}

vm_t sys_signal(vm_t pid, vm_t signal, vm_t u0, vm_t u1)
{
	UNUSED(u0); UNUSED(u1);
	/* TODO: signals? */
	return 0;
}

vm_t sys_switch(vm_t pid, vm_t u0, vm_t u1, vm_t u2)
{
	UNUSED(u0); UNUSED(u1); UNUSED(u2);
	/* TODO: switch to process */
	return 0;
}

vm_t sys_sync(vm_t buf, vm_t size, vm_t u0, vm_t u1)
{
	UNUSED(u0); UNUSED(u1);
	/* check that only the process manager can use this syscall, otherwise
	 * just dump process info into the buffer (I guess, not sure if this
	 * will be quite required */
	return 0;
}
