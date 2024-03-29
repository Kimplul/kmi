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
	vm_t start = 0;
	/** @todo expose flags to users */
	if (!(start = alloc_uvmem(r, size, flags)))
		return_args1(t, ERR_OOMEM);

	return_args2(t, OK, start);
}

/**
 * Allocate single page to program.
 *
 * @param t Current tcb.
 * @param size Size of the allocation.
 * @param flags Flags of allocation.
 * @return \ref ERR_OOMEM if unsucessful, otherwise \ref OK, virtual address,
 * actual size, physical address, in that order.
 */
SYSCALL_DEFINE2(req_page)(struct tcb *t, sys_arg_t size, sys_arg_t flags)
{
	struct tcb *r = get_cproc(t);
	vm_t start = 0; pm_t paddr = 0; size_t asize = size;
	if (!(start = alloc_uvpage(r, asize, flags, &asize, &paddr)))
		return_args1(t, ERR_OOMEM);

	return_args4(t, OK, start, asize, paddr);
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
	vm_t start = 0;
	if (!(start = alloc_fixed_uvmem(r, fixed, size, flags)))
		return_args1(t, ERR_OOMEM);

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
	vm_t vm_start = (vm_t)start;

	stat_t status = OK;
	/* try freeing normal user memory first, if that fails, try device
	 * memory, otherwise just assume the address is borked. */
	if (!(status = free_uvmem(r, vm_start)))
		return_args1(t, OK);

	if (!(status = free_devmem(r, vm_start)))
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
	vm_t start = 0;
	if (!(start = alloc_devmem(r, paddr, size, flags)))
		return_args1(t, ERR_OOMEM);

	return_args2(t, OK, start);
}

/**
 * Request shared memory syscall handler.
 *
 * @param t Current tcb.
 * @param tid Thread to share memory with.
 * @param size Minimum size of allocation.
 * @param sflags Flags of allocation for \p t.
 * @param cflags Flags of allocation for \p tid.
 * @return \ref OK and start of \p t allocation and start of \p tid allocation,
 * in that order, \ref ERR_OOMEM otherwise.
 *
 * @todo should we also take the thread who should get the other end of the
 * memory?
 */
SYSCALL_DEFINE4(req_sharedmem)(struct tcb *t, sys_arg_t tid,
                               sys_arg_t size, sys_arg_t sflags,
                               sys_arg_t cflags)
{
	/** @todo check capability for shared memory */
	struct tcb *u = get_tcb(tid);
	if (!u)
		return_args1(t, ERR_INVAL);

	struct tcb *s = get_cproc(t);
	struct tcb *c = get_rproc(u);

	vm_t sstart, cstart;
	if (alloc_shared_uvmem(s, c, size, sflags, cflags, &sstart, &cstart))
		return_args1(t, ERR_OOMEM);

	return_args3(t, OK, sstart, cstart);
}

/** \todo add some way to specify who gets to access the shared memory? */
