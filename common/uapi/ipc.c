#include <apos/uapi.h>
#include <apos/tcb.h>

vm_t sys_ipc_server(vm_t callback, vm_t u0, vm_t u1, vm_t u2)
{
	UNUSED(u0); UNUSED(u1); UNUSED(u2);
	struct tcb *r = cur_tcb();
	if(r->callback) /* server can't be reinitialized */
		return 1;

	r->callback = callback;
	return 0;
}

vm_t sys_ipc_req(vm_t pid, vm_t d0, vm_t d1, vm_t d2)
{
	struct tcb *t = get_tcb(pid);
	/* something like jump_to_callback(t, d0, d1, d2) */
	return 0;
}

vm_t sys_ipc_resp(vm_t pid, vm_t ret, vm_t u0, vm_t u1)
{
	UNUSED(u0); UNUSED(u1);
	struct tcb *r = get_tcb(pid);
	/* something like return_from_callback(t, r) */
	return 0; /* oh yeah probably unreachable? */
}
