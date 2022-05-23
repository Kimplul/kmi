/**
 * @file dispatch.c
 * Syscall dispatch.
 */

#include <apos/uapi.h>

static const sys_t syscall_table[] = {
	/* noop */
	[SYS_NOOP] = sys_noop,
	/* mem */
	[SYS_REQ_MEM] = sys_req_mem,
	[SYS_REQ_PMEM] = sys_req_pmem,
	[SYS_REQ_FIXMEM] = sys_req_fixmem,
	[SYS_FREE_MEM] = sys_free_mem,

	/* timers */
	[SYS_TIMEBASE] = sys_timebase,
	[SYS_REQ_REL_TIMER] = sys_req_rel_timer,
	[SYS_REQ_ABS_TIMER] = sys_req_abs_timer,
	[SYS_FREE_TIMER] = sys_free_timer,

	/* ipc */
	[SYS_IPC_SERVER] = sys_ipc_server,
	[SYS_IPC_REQ_PROC] = sys_ipc_req_proc,
	[SYS_IPC_REQ_THREAD] = sys_ipc_req_thread,
	[SYS_IPC_RESP] = sys_ipc_resp,

	/* proc */
	[SYS_CREATE] = sys_create,
	[SYS_FORK] = sys_fork,
	[SYS_EXEC] = sys_exec,
	[SYS_SIGNAL] = sys_signal,
	[SYS_SWAP] = sys_swap,

	/* conf */
	[SYS_CONF] = sys_conf,
	[SYS_POWEROFF] = sys_poweroff,
};

SYSCALL_DEFINE0(noop)(){
	return (struct sys_ret){ OK, 0 };
}

struct sys_ret syscall_dispatch(sys_arg_t syscall, sys_arg_t a, sys_arg_t b,
                                sys_arg_t c, sys_arg_t d)
{
	sys_t call = syscall_table[syscall];

	if (!call)
		return (struct sys_ret){ ERR_INVAL, 0 };

	return call(a, b, c, d);
}
