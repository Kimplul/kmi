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

SYSCALL_DEFINE4(ipc_req)
(sys_arg_t pid, sys_arg_t d0, sys_arg_t d1, sys_arg_t d2)
{
	struct tcb *t = get_tcb(pid);
	/* something like jump_to_callback(t, d0, d1, d2) */
	return (struct sys_ret){ OK, 0 };
}

SYSCALL_DEFINE3(ipc_resp)(sys_arg_t pid, sys_arg_t ret, sys_arg_t val)
{
	struct tcb *r = get_tcb(pid);
	/* something like return_from_callback(t, r) */
	return (struct sys_ret){ ret, val };
}
