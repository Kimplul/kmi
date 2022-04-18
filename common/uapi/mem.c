#include <apos/uapi.h>
#include <apos/utils.h>
#include <apos/vmem.h>
#include <apos/dmem.h>

SYSCALL_DEFINE2(req_mem)(vm_t size, vm_t flags)
{
	/* proc_tcb should give the tcb of the TID currently running */
	struct tcb *r = cur_tcb();
	return alloc_uvmem(r, size, flags);
}

SYSCALL_DEFINE3(req_fixmem)(vm_t start, vm_t size, vm_t flags)
{
	struct tcb *r = cur_tcb();
	return alloc_fixed_uvmem(r, start, size, flags);
}

SYSCALL_DEFINE1(free_mem)(vm_t start)
{
	struct tcb *r = cur_tcb();
	if (start > __pre_top && start < __post_base)
		free_uvmem(r, start);
	else
		free_devmem(r, start);

	return 0;
}

SYSCALL_DEFINE3(req_pmem)(vm_t paddr, vm_t size, vm_t flags)
{
	/* this will require some pondering, but essentially this syscall should
	 * only be used for device access, so any addresses requested should be
	 * outside the RAM area, and I'll probably have to implement some method
	 * that keeps track of used regions outside of RAM. We'll see.
	 */
	struct tcb *r = cur_tcb();
	return alloc_devmem(r, paddr, size, flags);
}

SYSCALL_DEFINE4(req_sharedmem)(vm_t pid, vm_t start, vm_t size, vm_t flags)
{
	/* take memory in PID's vaddr and map it somewhere in our own memory
	 * region.
	 */
	return 0;
}
