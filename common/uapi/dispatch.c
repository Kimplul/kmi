#include <apos/uapi.h>

static const sys_t syscall_table[] = {
	/* mem */
	[SYS_REQ_MEM] = sys_req_mem,
	[SYS_REQ_PMEM] = sys_req_pmem,
	[SYS_REQ_FIXMEM] = sys_req_fixmem,
	[SYS_FREE_MEM] = sys_free_mem,

	/* timers */
	[SYS_REQ_REL_TIMER] = sys_req_rel_timer,
	[SYS_REQ_ABS_TIMER] = sys_req_abs_timer,
	[SYS_FREE_TIMER] = sys_free_timer,

	/* ipc */
	[SYS_IPC_SERVER] = sys_ipc_server,
	[SYS_IPC_REQ] = sys_ipc_req,
	[SYS_IPC_RESP] = sys_ipc_resp,

	/* proc */
	[SYS_FORK] = sys_fork,
	[SYS_EXEC] = sys_exec,
	[SYS_SIGNAL] = sys_signal,
	[SYS_SWITCH] = sys_switch,
	[SYS_SYNC] = sys_sync,

	/* conf */
	[SYS_CONF] = sys_conf,
	[SYS_POWEROFF] = sys_poweroff,
};

vm_t syscall_dispatch(vm_t syscall, vm_t a, vm_t b, vm_t c, vm_t d)
{
	sys_t call = syscall_table[syscall];
	if (!call)
		return ERR_INVAL;

	return call(a, b, c, d);
}
