#include <apos/uapi.h>
#include <apos/utils.h>
#include <apos/vmem.h>

vm_t sys_req_mem(vm_t size, vm_t flags, vm_t u0, vm_t u1)
{
	UNUSED(u0); UNUSED(u1);
	/* proc_tcb should give the tcb of the TID currently running */
	struct tcb *r = cur_tcb();
	return alloc_uvmem(r, size, flags);
}

vm_t sys_req_fixmem(vm_t start, vm_t size, vm_t flags, vm_t u0)
{
	UNUSED(u0);
	struct tcb *r = cur_tcb();
	return alloc_fixed_uvmem(r, start, size, flags);
}

vm_t sys_free_mem(vm_t start, vm_t u0, vm_t u1, vm_t u2)
{
	UNUSED(u0); UNUSED(u1); UNUSED(u2);
	struct tcb *r = cur_tcb();
	free_uvmem(r, start);
	return 0;
}

vm_t sys_req_pmem(vm_t paddr, vm_t size, vm_t flags, vm_t u0)
{
	UNUSED(u0);
	/* this will require some pondering, but essentially this syscall should
	 * only be used for device access, so any addresses requested should be
	 * outside the RAM area, and I'll probably have to implement some method
	 * that keeps track of used regions outside of RAM. We'll see.
	 */
	return 0;
}

vm_t sys_req_sharedmem(vm_t pid, vm_t start, vm_t size, vm_t flags)
{
	/* take memory in PID's vaddr and map it somewhere in our own memory
	 * region.
	 */
	return 0;
}
