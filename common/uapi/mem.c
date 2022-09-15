/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file mem.c
 * Memory handling syscall implementations.
 * \todo Should we return more error information?
 */

#include <apos/uapi.h>
#include <apos/utils.h>
#include <apos/vmem.h>
#include <apos/dmem.h>

/**
 * Memory request syscall handler.
 *
 * @param size Minimum size of allocation.
 * @param flags Flags of allocation.
 * @return \ref OK and start of allocation when succesful,
 * \ref ERR_OOMEM and \c NULL otherwise.
 */
SYSCALL_DEFINE2(req_mem)(sys_arg_t size, sys_arg_t flags)
{
	/* get current effective process */
	struct tcb *r = cur_proc();
	vm_t start = 0;
	if ((start = alloc_uvmem(r, size, flags)))
		return (struct sys_ret){ ERR_OOMEM, NULL, 0, 0, 0, 0 };

	return (struct sys_ret){ OK, start, 0, 0, 0, 0 };
}

/**
 * Fixed memory request syscall handler.
 *
 * @param fixed Address which should be included in allocation.
 * @param size Minimum size of allocation after \c start.
 * @param flags Flags of allocation.
 * @return \ref OK and start of allocation when succesful,
 * \ref ERR_OOMEM and \c NULL otherwise.
 */
SYSCALL_DEFINE3(req_fixmem)(sys_arg_t fixed, sys_arg_t size, sys_arg_t flags)
{
	struct tcb *r = cur_proc();
	vm_t start = 0;
	if ((start = alloc_fixed_uvmem(r, fixed, size, flags)))
		return (struct sys_ret){ ERR_OOMEM, NULL, 0, 0, 0, 0 };

	return (struct sys_ret){ OK, start, 0, 0, 0, 0 };
}

/**
 * Free memory syscall handler.
 *
 * @param start Start of allocation to free.
 * @return \ref OK and \c 0 when succesful, \ref ERR_NF and \c 0 otherwise.
 */
SYSCALL_DEFINE1(free_mem)(sys_arg_t start)
{
	struct tcb *r = cur_proc();
	vm_t vm_start = (vm_t)start;

	stat_t status = OK;
	if (vm_start > __pre_top && vm_start < __post_base)
		status = free_uvmem(r, vm_start);
	else
		status = free_devmem(r, vm_start);

	if (status)
		return (struct sys_ret){ ERR_NF, 0, 0, 0, 0, 0 };

	return (struct sys_ret){ OK, 0, 0, 0, 0, 0 };
}

/**
 * Request physical memory syscall handler.
 *
 * @param paddr Physical address to map.
 * @param size Minimum size of allocation.
 * @param flags Flags of allocation.
 * @return \ref OK and start of allocation when succesful,
 * \ref ERR_OOMEM and \c NULL otherwise.
 */
SYSCALL_DEFINE3(req_pmem)(sys_arg_t paddr, sys_arg_t size, sys_arg_t flags)
{
	/* this will require some pondering, but essentially this syscall should
	 * only be used for device access, so any addresses requested should be
	 * outside the RAM area, and I'll probably have to implement some method
	 * that keeps track of used regions outside of RAM. We'll see.
	 */
	struct tcb *r = cur_proc();
	vm_t start = 0;
	if ((start = alloc_devmem(r, paddr, size, flags)))
		return (struct sys_ret){ ERR_OOMEM, NULL, 0, 0, 0, 0 };

	return (struct sys_ret){ OK, start, 0, 0, 0, 0 };
}

/**
 * Request shared memory syscall handler.
 *
 * @param size Minimum size of allocation.
 * @param flags Flags of allocation.
 * @return \ref OK and start of allocation when succesful,
 * \ref ERR_OOMEM and \c NULL otherwise.
 */
SYSCALL_DEFINE2(req_sharedmem)(sys_arg_t size, sys_arg_t flags)
{
	/** \todo check that requester is server */
	struct tcb *t = cur_proc();
	vm_t start = 0;
	if ((start = alloc_shared_uvmem(t, size, flags)))
		return (struct sys_ret){ ERR_OOMEM, NULL, 0, 0, 0, 0 };

	return (struct sys_ret){ OK, start, 0, 0, 0, 0 };
}

/**
 * Reference shared memory syscall handler.
 *
 * @param tid Thread ID of shared memory owner.
 * @param va Start of shared memory in \c tid.
 * @param flags Flags of reference.
 * @return \ref OK and start of reference when succesful,
 * \ref ERR_OOMEM and \c NULL otherwise.
 */
SYSCALL_DEFINE3(ref_sharedmem)(sys_arg_t tid, sys_arg_t va, sys_arg_t flags)
{
	struct tcb *t1 = cur_tcb();
	struct tcb *t2 = get_tcb(tid);
	vm_t start = 0;
	if ((start = ref_shared_uvmem(t1, t2, va, flags)))
		return (struct sys_ret){ ERR_OOMEM, NULL, 0, 0, 0, 0 };

	return (struct sys_ret){ OK, start, 0, 0, 0, 0 };
}

/** \todo add some way to specify who gets to access the shared memory? */
