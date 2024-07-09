/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file mem.c
 * Memory handling syscall implementations.
 * \todo Should we return more error information?
 */

#include <kmi/uapi.h>
#include <kmi/utils.h>
#include <kmi/vmem.h>
#include <kmi/dmem.h>

/**
 * Memory request syscall handler.
 *
 * @param t Current tcb.
 * @param size Minimum size of allocation.
 * @param flags Flags of allocation.
 * @return \ref OK and start of allocation when succesful,
 * \ref ERR_OOMEM and \c NULL otherwise.
 */
SYSCALL_DEFINE2(req_mem)(struct tcb *t, sys_arg_t size, sys_arg_t flags)
{
	struct tcb *r = get_cproc(t);
	flags = sanitize_uvflags(flags);
	vm_t start = alloc_uvmem(r, size, flags);
	if (ERR_CODE(start))
		return_args1(t, start);

	return_args2(t, OK, start);
}


/**
 * Fixed memory request syscall handler.
 *
 * @param t Current tcb.
 * @param fixed Address which should be included in allocation.
 * @param size Minimum size of allocation after \c start.
 * @param flags Flags of allocation.
 * @return \ref OK and start of allocation when succesful,
 * \ref ERR_OOMEM and \c NULL otherwise.
 */
SYSCALL_DEFINE3(req_fixmem)(struct tcb *t, sys_arg_t fixed, sys_arg_t size,
                            sys_arg_t flags)
{
	struct tcb *r = get_cproc(t);
	flags = sanitize_uvflags(flags);
	vm_t start = alloc_fixed_uvmem(r, fixed, size, flags);
	if (ERR_CODE(start))
		return_args1(t, start);

	return_args2(t, OK, start);
}

/**
 * Free memory syscall handler.
 *
 * @param t Current tcb.
 * @param start Start of allocation to free.
 * @return \ref OK and \c 0 when succesful, \ref ERR_NF and \c 0 otherwise.
 */
SYSCALL_DEFINE1(free_mem)(struct tcb *t, sys_arg_t start)
{
	struct tcb *r = get_cproc(t);

	stat_t status = OK;
	/* try freeing normal user memory first, if that fails, try device
	 * memory, otherwise just assume the address is borked. */
	if (!(status = free_uvmem(r, start)))
		return_args1(t, OK);

	if (!(status = free_devmem(r, start)))
		return_args1(t, OK);

	return_args1(t, status);
}

/**
 * Request physical memory syscall handler.
 *
 * @param t Current tcb.
 * @param paddr Physical address to map.
 * @param size Minimum size of allocation.
 * @param flags Flags of allocation.
 * @return \ref OK and start of allocation when succesful,
 * \ref ERR_OOMEM and \c NULL otherwise.
 */
SYSCALL_DEFINE3(req_pmem)(struct tcb *t, sys_arg_t paddr, sys_arg_t size,
                          sys_arg_t flags)
{
	/* this will require some pondering, but essentially this syscall should
	 * only be used for device access, so any addresses requested should be
	 * outside the RAM area, and I'll probably have to implement some method
	 * that keeps track of used regions outside of RAM. We'll see.
	 */
	struct tcb *r = get_cproc(t);
	flags = sanitize_uvflags(flags);
	vm_t start = alloc_devmem(r, paddr, size, flags);
	if (ERR_CODE(start))
		return_args1(t, start);

	return_args2(t, OK, start);
}

/**
 * Allocate some contiguous region in memory.
 * Due to page allocation constraints, this is generally just one page that's
 * larger than the \p size parameter. Should work fine for 4K - 2M allocations,
 * which I assume is generally what is used, but we'll see if I have to
 * implement an actually good page allocator at some point.
 *
 * @param t Current tcb.
 * @param size Minimum size of mapping.
 * @param flags Flags of mapping. Unsure why you'd want anything besides VM_W |
 * VM_R, but eh.
 *
 * @return \see sys_req_page().
 */
SYSCALL_DEFINE2(req_page)(struct tcb *t, sys_arg_t size, sys_arg_t flags)
{
	struct tcb *r = get_cproc(t);

	pm_t addr = 0;
	vm_t start = 0;
	size_t asize = 0;
	flags = sanitize_uvflags(flags);
	start = alloc_uvpage(r, size, flags, &addr, &asize);
	if (ERR_CODE(start))
		return_args1(t, start);

	return_args4(t, OK, start, addr, asize);
}

/**
 * Request shared memory syscall handler.
 *
 * @param t Current tcb.
 * @param size Minimum size of allocation.
 * @param flags Flags of allocation.
 * @return \ref OK, start
 * in that order, \ref ERR_OOMEM otherwise.
 */
SYSCALL_DEFINE2(req_sharedmem)(struct tcb *t, sys_arg_t size, sys_arg_t flags)
{
	struct tcb *c = get_cproc(t);
	if (!has_cap(c->caps, CAP_SHARED))
		return_args1(t, ERR_PERM);

	flags = sanitize_uvflags(flags);
	vm_t start = alloc_shared_uvmem(c, size, flags);
	if (ERR_CODE(start))
		return_args1(t, start);

	return_args2(t, OK, start);
}

/**
 * Reference shared memory.
 *
 * @param t Current tcb.
 * @param tid In which thread's address space to create mapping.
 * @param addr Address of shared region in \p t.
 * @param flags Flags of allocation.
 * @return \ref ERR_OOMEM if unsucessful, otherwise \ref OK, virtual address,
 * actual size, in that order. Generally the actual size should match with the
 * original shared region, but I wouldn't count on it.
 */
SYSCALL_DEFINE3(ref_sharedmem)(struct tcb *t, sys_arg_t tid, sys_arg_t addr,
                               sys_arg_t flags)
{
	struct tcb *c = get_cproc(t);
	if (!has_cap(c->caps, CAP_SHARED))
		return_args1(t, ERR_PERM);

	struct tcb *r = get_tcb(tid);
	if (!r || zombie(r))
		return_args1(t, ERR_INVAL);

	flags = sanitize_uvflags(flags);
	vm_t start = ref_shared_uvmem(r, c, addr, flags);
	if (ERR_CODE(start))
		return_args1(t, start);

	return_args2(t, OK, start);
}
