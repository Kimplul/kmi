#include <apos/uapi.h>
#include <apos/utils.h>
#include <apos/vmem.h>
#include <apos/dmem.h>

SYSCALL_DEFINE2(req_mem)(sys_arg_t size, sys_arg_t flags)
{
	/* proc_tcb should give the tcb of the TID currently running */
	struct tcb *r = cur_tcb();
	return (struct sys_ret){ OK, alloc_uvmem(r, size, flags) };
}

SYSCALL_DEFINE3(req_fixmem)(sys_arg_t start, sys_arg_t size, sys_arg_t flags)
{
	struct tcb *r = cur_tcb();
	/* should probably check if the allocation succeeded...? TODO */
	return (struct sys_ret){ OK, alloc_fixed_uvmem(r, start, size, flags) };
}

SYSCALL_DEFINE1(free_mem)(sys_arg_t start)
{
	struct tcb *r = cur_tcb();
	vm_t vm_start = (vm_t)start;

	if (vm_start > __pre_top && vm_start < __post_base)
		free_uvmem(r, vm_start);
	else
		free_devmem(r, vm_start);

	return (struct sys_ret){ OK, 0 };
}

SYSCALL_DEFINE3(req_pmem)(sys_arg_t paddr, sys_arg_t size, sys_arg_t flags)
{
	/* this will require some pondering, but essentially this syscall should
	 * only be used for device access, so any addresses requested should be
	 * outside the RAM area, and I'll probably have to implement some method
	 * that keeps track of used regions outside of RAM. We'll see.
	 */
	struct tcb *r = cur_tcb();
	return (struct sys_ret){ OK, alloc_devmem(r, paddr, size, flags) };
}

SYSCALL_DEFINE4(req_sharedmem)
(sys_arg_t tid, sys_arg_t start, sys_arg_t size, sys_arg_t flags)
{
	/* called by server, take memory allocation at start and map it into
	 * tid's address space? */
	return (struct sys_ret){ OK, 0 };
}
