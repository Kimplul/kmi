/**
 * @file ipc.c
 * Interprocess communication syscall implementations.
 */

#include <apos/uapi.h>
#include <apos/tcb.h>

SYSCALL_DEFINE1(ipc_server)(sys_arg_t callback)
{
	struct tcb *r = cur_tcb();
	if (r->callback) /* server can't be reinitialized */
		return (struct sys_ret){ ERR_EXT, 0 };

	r->callback = callback;
	return (struct sys_ret){ OK, 0 };
}

SYSCALL_DEFINE3(ipc_req_proc)(sys_arg_t pid, sys_arg_t d0, sys_arg_t d1)
{
	struct tcb *r = get_tcb(pid);
	/* TODO: something like jump_to_callback(t) */
	return (struct sys_ret){ d0, d1 };
}

SYSCALL_DEFINE3(ipc_req_thread)(sys_arg_t tid, sys_arg_t d0, sys_arg_t d1)
{
	struct tcb *t = get_tcb(tid);
	/* ditto */
	return (struct sys_ret){ d0, d1 };
}

SYSCALL_DEFINE2(ipc_resp)(sys_arg_t d0, sys_arg_t d1)
{
	struct tcb *r = cur_tcb();
	/* something like return_from_callback(t, r) */
	return (struct sys_ret){ d0, d1 };
}
