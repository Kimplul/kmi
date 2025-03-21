/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_UAPI_H
#define KMI_UAPI_H

/**
 * @file uapi.h
 * Userspace api, syscall declarations.
 */

#include <kmi/syscalls.h>
#include <kmi/vmem.h>

/**
 * Syscall function type.
 * Let's start with five arguments and see where that goes
 */
typedef void (*sys_t)(struct tcb *t, long, long, long, long, long);

/** Helper for returning sys_ret with 0 arguments. */
#define SYS_RET0() (struct sys_ret){0, 0, 0, 0, 0, 0}

/** Helper for returning sys_ret with 1 arguments. */
#define SYS_RET1(a) (struct sys_ret){a, 0, 0, 0, 0, 0}

/** Helper for returning sys_ret with 2 arguments. */
#define SYS_RET2(a, b) (struct sys_ret){a, b, 0, 0, 0, 0}

/** Helper for returning sys_ret with 3 arguments. */
#define SYS_RET3(a, b, c) (struct sys_ret){a, b, c, 0, 0, 0}

/** Helper for returning sys_ret with 4 arguments. */
#define SYS_RET4(a, b, c, d) (struct sys_ret){a, b, c, d, 0, 0}

/** Helper for returning sys_ret with 5 arguments. */
#define SYS_RET5(a, b, c, d, e) (struct sys_ret){a, b, c, d, e, 0}

/** Helper for returning sys_ret with 6 arguments. */
#define SYS_RET6(a, b, c, d, e, f) (struct sys_ret){a, b, c, d, e, f}

/**
 * Helper macro for declaring syscalls with zero arguments.
 *
 * The idea is that all syscalls externally have the same amount of arguments,
 * but then by using some helper macros (see below) we can make actually
 * implementing the syscall more intuitive and less noisy.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DECLARE0(name)         \
	void sys_##name(struct tcb *t, \
			sys_arg_t a,   \
			sys_arg_t b,   \
			sys_arg_t c,   \
			sys_arg_t d,   \
			sys_arg_t e);

/**
 * Helper macro for declaring syscalls with one argument.
 *
 * @param name Name of syscall.
 * @param a Name of argument.
 */
#define SYSCALL_DECLARE1(name, a)                                \
	void sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, \
			sys_arg_t c,                             \
			sys_arg_t d, sys_arg_t e);

/**
 * Helper macro for declaring syscalls with two arguments.
 *
 * @param name Name of syscall.
 * @param a Name of first argument.
 * @param b Name of second argument.
 */
#define SYSCALL_DECLARE2(name, a, b)                             \
	void sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, \
			sys_arg_t c,                             \
			sys_arg_t d, sys_arg_t e);

/**
 * Helper macro for declaring syscalls with three arguments.
 *
 * @param name Name of syscall.
 * @param a Name of first argument.
 * @param b Name of second argument.
 * @param c Name of third argument.
 */
#define SYSCALL_DECLARE3(name, a, b, c)                          \
	void sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, \
			sys_arg_t c,                             \
			sys_arg_t d, sys_arg_t e);

/**
 * Helper macro for declaring syscalls with four arguments.
 *
 * @param name Name of syscall.
 * @param a Name of first argument.
 * @param b Name of second argument.
 * @param c Name of third argument.
 * @param d Name of fourth argument.
 */
#define SYSCALL_DECLARE4(name, a, b, c, d)                       \
	void sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, \
			sys_arg_t c,                             \
			sys_arg_t d, sys_arg_t e);

/**
 * Helper macro for declaring syscalls with five arguments.
 *
 * @param name Name of syscall.
 * @param a Name of first argument.
 * @param b Name of second argument.
 * @param c Name of third argument.
 * @param d Name of fourth argument.
 * @param e Name of fifth argument.
 */
#define SYSCALL_DECLARE5(name, a, b, c, d, e)                    \
	void sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, \
			sys_arg_t c,                             \
			sys_arg_t d, sys_arg_t e);

/**
 * Helper macro for defining syscall with zero arguments.
 *
 * Note the \c __noinline attribute, might seem weird but turned out
 * that gcc liked to inline all system calls into handle_syscall(),
 * which seems to add quite a bit of moving registers around and spilling
 * into memory. Forcing the syscalls to not be inlined removed this
 * overhead and sped things up by ~7 %.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE0(name)                                               \
	static inline void __##name(struct tcb *t);                         \
	void __noinline sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, \
				   sys_arg_t c,                             \
				   sys_arg_t d, sys_arg_t e)                \
	{                                                                   \
		UNUSED(a);                                                  \
		UNUSED(b);                                                  \
		UNUSED(c);                                                  \
		UNUSED(d);                                                  \
		UNUSED(e);                                                  \
		__##name(t);                                                \
	}                                                                   \
	static inline void __##name

/**
 * Helper macro for defining syscall with one argument.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE1(name)                                               \
	static inline void __##name(struct tcb *, sys_arg_t);               \
	void __noinline sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, \
				   sys_arg_t c,                             \
				   sys_arg_t d, sys_arg_t e)                \
	{                                                                   \
		UNUSED(b);                                                  \
		UNUSED(c);                                                  \
		UNUSED(d);                                                  \
		UNUSED(e);                                                  \
		__##name(t, a);                                             \
	}                                                                   \
	static inline void __##name

/**
 * Helper macro for defining syscall with two arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE2(name)                                               \
	static inline void __##name(struct tcb *, sys_arg_t,                \
				    sys_arg_t);                             \
	void __noinline sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, \
				   sys_arg_t c,                             \
				   sys_arg_t d, sys_arg_t e)                \
	{                                                                   \
		UNUSED(c);                                                  \
		UNUSED(d);                                                  \
		UNUSED(e);                                                  \
		__##name(t, a, b);                                          \
	}                                                                   \
	static inline void __##name

/**
 * Helper macro for defining syscall with three arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE3(name)                                               \
	static inline void __##name(struct tcb *, sys_arg_t,                \
				    sys_arg_t,                              \
				    sys_arg_t);                             \
	void __noinline sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, \
				   sys_arg_t c,                             \
				   sys_arg_t d, sys_arg_t e)                \
	{                                                                   \
		UNUSED(d);                                                  \
		UNUSED(e);                                                  \
		__##name(t, a, b, c);                                       \
	}                                                                   \
	static inline void __##name

/**
 * Helper macro for defining syscall with four arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE4(name)                                               \
	static inline void __##name(struct tcb *, sys_arg_t,                \
				    sys_arg_t, sys_arg_t,                   \
				    sys_arg_t);                             \
	void __noinline sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, \
				   sys_arg_t c,                             \
				   sys_arg_t d, sys_arg_t e)                \
	{                                                                   \
		UNUSED(e);                                                  \
		__##name(t, a, b, c, d);                                    \
	}                                                                   \
	static inline void __##name

/**
 * Helper macro for defining syscall with five arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE5(name)                                               \
	static inline void __##name(struct tcb *, sys_arg_t,                \
				    sys_arg_t, sys_arg_t,                   \
				    sys_arg_t, sys_arg_t);                  \
	void __noinline sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, \
				   sys_arg_t c,                             \
				   sys_arg_t d, sys_arg_t e)                \
	{                                                                   \
		__##name(t, a, b, c, d, e);                                 \
	}                                                                   \
	static void __##name

/** @name Misc syscalls. */
/** @{ */

/**
 * Noop syscall.
 *
 * @param t Current tcb.
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and 0.
 */
SYSCALL_DECLARE0(noop);

/**
 * Putch syscall.
 *
 * @param t Current tcb.
 * @param ch Character to put.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and 0.
 */
SYSCALL_DECLARE1(putch, ch);

/** @} */

/* @name Memory handling syscalls. */
/** @{ */
/**
 * Request memory syscall.
 *
 * Allocates at least the specified size of allocation to current effective
 * process.
 *
 * @param t Current tcb.
 * @param size Size of allocation.
 * @param flags Flags of allocation.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and start of memory allocation.
 */
SYSCALL_DECLARE2(req_mem, size, flags);

/**
 * Reference shared memory, i.e. create a mapping for it in \p tid.
 *
 * @param t Current tcb, owner of \p addr.
 * @param tid Thread to create mapping in.
 * @param addr Address of shared region in \p t.
 * @param flags Mapping flags to use.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK, virtual address, size, in that order.
 */
SYSCALL_DECLARE3(ref_sharedmem, tid, addr, flags);

/**
 * Request physical memory syscall.
 *
 * Allocates at least the specified size of allocation which includes the
 * physical start address somewhere in the allocation to the current effective
 * process.
 *
 * @param t Current tcb.
 * @param paddr Start of physical allocation.
 * @param size Size of physical allocation.
 * @param flags Flags of physical allocation.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and start of memory allocation.
 */
SYSCALL_DECLARE3(req_pmem, paddr, size, flags);

/**
 * Get physically contiguous region of memory, useful for stuff like virtio
 * buffers.
 *
 * @param t Current tcb.
 * @param size Minimum size of region.
 * @param flags Flags of allocation.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Return\ref OK and start of virtual address, start of physical address and
 * size, in that order.
 *
 * @note Users are encouraged to check the return size, as it may be
 * significantly larger than requested, in which case it is recommended to place
 * multiple buffers in the region if required by the application.
 */
SYSCALL_DECLARE2(req_page, size, flags);

/**
 * Request fixed memory syscall.
 *
 * Allocates at least the specified size of allocation which includes the start
 * address of allocation to the current effective process.
 *
 * @param t Current tcb.
 * @param start Start of allocation.
 * @param size Size of allocation.
 * @param flags Flags of allocation.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and start of memory allocation.
 */
SYSCALL_DECLARE3(req_fixmem, start, size, flags);

/**
 * Request shared memory syscall.
 *
 * Allocates a region that can be shared between different processes, that is at
 * least the specified size of allocation. When clients are freeing memory, the
 * underlying physical allocation will not be freed unless the owning reference
 * (server) frees it.
 *
 * @param t Current tcb.
 * @param tid Thread to share memory with.
 * @param size Size of allocation.
 * @param sflags Flags of allocation for server, that is \p t.
 * @param cflags Flags of allocation for client, that is \p tid.
 * @param e Unused.
 *
 * Returns \ref OK and start of memory allocation.
 */
SYSCALL_DECLARE4(req_sharedmem, tid, size, sflags, cflags);

/**
 * Free memory syscall.
 *
 * Frees the memory region pointed to.
 *
 * @param t Current tcb.
 * @param start Start of memory.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and 0.
 */
SYSCALL_DECLARE1(free_mem, start);
/** @} */

/** @name Timer syscalls. */
/** @{ */
/**
 * Get timer accuracy in Hertz syscall.
 *
 * @param t Current tcb.
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and frequency.
 * \todo Should this be some kind of config request instead of a separate
 * syscall?
 */
SYSCALL_DECLARE0(timebase);

/**
 * Get current ticks.
 *
 * @param t Current tcb.
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns current ticks.
 */
SYSCALL_DECLARE0(ticks);

/**
 * Request relative timer syscall.
 *
 * Request timer that triggers a number of ticks in the future.
 *
 * @param t Current tcb.
 * @param ticks Number of ticks from now.
 * @param mult Multiplier for \p ticks if we're on 32bit systems to produce a
 * combined 64 bit value. Ignored on 64bit systems.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and ID of timer.
 */
SYSCALL_DECLARE2(req_rel_timer, ticks, mult);

/**
 * Request absolute timer syscall.
 *
 * Request timer that triggers at some absolute timepoint.
 *
 * @param t Current tcb.
 * @param ticks Timepoint.
 * @param mult Multiplier.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and ID of timer.
 * \todo Check repeat value.
 */
SYSCALL_DECLARE2(req_abs_timer, ticks, mult);

/**
 * Free timer syscall.
 *
 * @param t Current tcb.
 * @param cid ID of timer to free.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK.
 */
SYSCALL_DECLARE1(free_timer, cid);
/** @} */

/** @name IPC syscalls. */
/** @{ */
/**
 * Request syscall.
 *
 * @param t Current tcb.
 * @param pid Request target process.
 * @param d0 First request argument.
 * @param d1 Second request argument.
 * @param d2 Third forwarding argument.
 * @param d3 Fourth forwarding argument.
 *
 * Returns \ref OK and whatever the server sends back on success, otherwise some
 * other status value.
 */
SYSCALL_DECLARE5(ipc_req, pid, d0, d1, d2, d3);

/**
 * Forwarding syscall.
 *
 * In a server request handler, do a request to some other server on behalf of
 * whoever called us up.
 *
 * @param t Current tcb.
 * @param pid Forwarding target process.
 * @param d0 First forwarding argument.
 * @param d1 Second forwarding argument.
 * @param d2 Third forwarding argument.
 * @param d3 Fourth forwarding argument.
 *
 * Returns \ref OK and whatever the server sends back on success, otherwise some
 * other status value.
 */
SYSCALL_DECLARE5(ipc_fwd, pid, d0, d1, d2, d3);

/**
 * Tail syscall.
 *
 * Requests that the current process doesn't get returned to in an rpc stack.
 *
 * @param t Current tcb.
 * @param pid Tail target process.
 * @param d0 First argument.
 * @param d1 Second argument.
 * @param d2 Third argument.
 * @param d3 Fourth argument.
 */
SYSCALL_DECLARE5(ipc_tail, pid, d0, d1, d2, d3);

/**
 * Kicking syscall.
 * Kicks the handling of an IPC  to someone else, jumping over the
 * current process at ipc_resp(). Effectively does a forward and a tail at the
 * same time.
 *
 * I'm imagining that this is useful in cases where an init process connects a
 * client to another server, and kicks the actual request handling to the
 * server. Note that ipc_resp() returning the ID of the process that handled a
 * request is useful in case the client and server should continue communicating
 * with eachother instead of going through init every time.
 *
 * Otherwise identical to ipc_fwd().
 *
 * @param t Current tcb.
 * @param pid Process to kick req/fwd to.
 * @param d0 First argument.
 * @param d1 Second argument.
 * @param d2 Third argument.
 * @param d3 Fourth argument.
 *
 * Doesn't return.
 */
SYSCALL_DECLARE5(ipc_kick, pid, d0, d1, d2, d3);

/**
 * Response syscall. If the current rpc frame is a notification frame, the
 * arguments will be ignored, turned out to be easier to implement than try to
 * enforce an ipc_ghost(), especially in cases where the root process has
 * already been killed. Generally this shouldn't be an issue, as the
 * notification handler is always aware of when it's being called to handle a
 * notification, and can tailor its ipc_resp() to return zero arguments or
 * whatever.
 *
 * @param t Current tcb.
 * @param d0 First response argument.
 * @param d1 Second response argument.
 * @param d2 Third response argument.
 * @param d3 Fourth response argument.
 * @param e Unused.
 *
 * Returns \c d0 and \c d1.
 */
SYSCALL_DECLARE4(ipc_resp, d0, d1, d2, d3);

/**
 * Notify thread syscall.
 *
 * @param t Current tcb.
 * @param tid Thread ID to notify.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and 0.
 */
SYSCALL_DECLARE1(ipc_notify, tid);
/** @} */

/** @name Process handling syscalls. */
/** @{ */
/**
 * Create thread syscall.
 *
 * Creates thread in current effective process context.
 *
 * @param t Current tcb.
 * @param func Function to call on startup.
 * @param d0 Argument 0.
 * @param d1 Argument 1.
 * @param d2 Argument 2.
 * @param d3 Argument 3.
 *
 * Returns \ref OK and 0.
 * \todo Should this take stack size etc?
 */
SYSCALL_DECLARE5(create, func, d0, d1, d2, d3);

/**
 * Fork process syscall.
 *
 * Forks a process, much like in *nix systems.
 *
 * @param t Current tcb.
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and 0.
 */
SYSCALL_DECLARE0(fork);

/**
 * Execute binary syscall.
 *
 * Executes a new binary in existing process space.
 *
 * @param t Current tcb.
 * @param bin Address of binary.
 * @param interp Optional address of interpreter.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and 0.
 * \todo Check other return codes.
 */
SYSCALL_DECLARE2(exec, bin, interp);

/**
 * Spawn binary syscall. Try to prefer over unixy fork/exec.
 *
 * Executes a new binary in new process space.
 *
 * @param t Current tcb.
 * @param bin Address of binary.
 * @param interp Optional address of interpreter.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Retuns \ref OK and 0.
 * \todo Check other return codes.
 */
SYSCALL_DECLARE2(spawn, bin, interp);

/**
 * Swap syscall.
 *
 * Swap currently running thread.
 *
 * @param t Current tcb.
 * @param tid Thread to swap to.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and 0.
 */
SYSCALL_DECLARE1(swap, tid);

/** @} */

/** @name Configuration syscalls. */
/** @{ */
/**
 * Set configuration syscall.
 *
 * Set some runtime parameter.
 *
 * @param t Current tcb.
 * @param param Parameter to set.
 * @param val Value to set parameter to.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK and 0.
 */
SYSCALL_DECLARE2(set_conf, param, val);

/**
 * Get configuration syscall.
 *
 * Get some runtime parameter.
 *
 * @param t Current tcb.
 * @param param Parameter to get.
 * @param d0 Optional data argument to parameter.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK.
 */
SYSCALL_DECLARE2(get_conf, param, d0);

/**
 * Set capabilities.
 *
 * @param t Current tcb.
 * @param tid Thread ID whose capabilities to set.
 * @param caps Mask of capabilities to set.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK on success, \ref ERR_INVAL on invalid input.
 */
SYSCALL_DECLARE2(set_cap, tid, caps);

/**
 * Get capabilities.
 *
 * @param t Current tcb.
 * @param tid Thread ID whose capabilities to get.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK, capabilities.
 */
SYSCALL_DECLARE1(get_cap, tid);

/**
 * Clear capabilities.
 *
 * @param t Current tcb.
 * @param tid Thread ID whose capabilities to clear.
 * @param cap Mask of capabilities to clear.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK.
 */
SYSCALL_DECLARE2(clear_cap, tid, cap);

/**
 * Set which process to move notifications to for thread.
 * @todo Should converge on either calling everything irq or notification or
 * whatever, but mixing both is a bit dumb.
 *
 * @param t Current tcb.
 * @param tid Thread whose notifications should be moved.
 * @param pid Process that is willing to handle the notifications.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns \ref OK.
 */
SYSCALL_DECLARE2(set_handler, tid, pid);

/**
 * Power off syscall.
 *
 * Either shut down or reboot system.
 * At least currently, this should be called with care, requires that the caller has
 * the POWER capability bit. Does not ensure that all threads are shut down gracefully,
 * so caller should ensure that nothing else on the system is running when the
 * shutdown occurs.
 *
 * @param t Current tcb.
 * @param type Type of shutdown. \see poweroff_type.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Shouldn't return at all.
 */
SYSCALL_DECLARE1(poweroff, type);

/**
 * Sets the current core to sleep. Might take parameters in the future if there
 * arises a need for different levels of sleep, but for now zero params.
 *
 * @param t Current tcb.
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Shouldn't return at all.
 */
SYSCALL_DECLARE0(sleep);

/**
 * Request to handle IRQ.
 *
 * @param t Current tcb.
 * @param id ID of IRQ to handle.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 *
 * Returns OK on success, non-zero otherwise.
 * @todo document error codes better.
 */
SYSCALL_DECLARE1(irq_req, id);

/**
 * Remove handled IRQ.
 *
 * @param t Current tcb.
 * @param id ID of IRQ to stop handling.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 */
SYSCALL_DECLARE1(free_irq, id);

/**
 * Request that a thread exits, i.e. removes itself from the thread list and
 * frees all kernel data associated with thread.
 *
 * @param t Current tcb.
 * @param tid Thread to swap to once the current thread no longer exists. 0 for
 * sleeping.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 */
SYSCALL_DECLARE1(exit, tid);

/**
 * Request that a thread becomes orphant, i.e. eventually attached to the init
 * process. Can be used to stop threads within a process.
 *
 * @param t Current tcb.
 * @param tid Which thread to orphanize.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 */
SYSCALL_DECLARE1(detach, tid);

/** @} */

/**
 * Dispatch to correct syscall handler.
 *
 * @param syscall Syscall number.
 * @param a Syscall argument 0.
 * @param b Syscall argument 1.
 * @param c Syscall argument 2.
 * @param d Syscall argument 3.
 * @param e Syscall argument 4.
 * @param t Current tcb.
 * Returns whatever the specified syscall returns.
 */
void handle_syscall(sys_arg_t syscall, sys_arg_t a, sys_arg_t b,
                    sys_arg_t c, sys_arg_t d, sys_arg_t e, struct tcb *t);

/** \todo Should I add variable names as well, to make the documentation a bit
 * more readable? */

#include <arch/proc.h>

/**
 * Set one sys_ret value to pass to thread when returning to userspace.
 *
 * @param t Thread whose values to set.
 * @param a First value.
 */
#define set_ret1(t, a) set_ret(t, 1, SYS_RET1(a))

/**
 * Set two sys_ret values to pass to thread when returning to userspace.
 *
 * @param t Thread whose values to set.
 * @param a First value.
 * @param b Second value.
 */
#define set_ret2(t, a, b) set_ret(t, 2, SYS_RET2(a, b))

/**
 * Set three sys_ret values to pass to thread when returning to userspace.
 *
 * @param t Thread whose values to set.
 * @param a First value.
 * @param b Second value.
 * @param c Third value.
 */
#define set_ret3(t, a, b, c) set_ret(t, 3, SYS_RET3(a, b, c))

/**
 * Set four sys_ret values to pass to thread when returning to userspace.
 *
 * @param t Thread whose values to set.
 * @param a First value.
 * @param b Second value.
 * @param c Third value.
 * @param d Fourth value.
 */
#define set_ret4(t, a, b, c, d) set_ret(t, 4, SYS_RET4(a, b, c, d))

/**
 * Set five sys_ret values to pass to thread when returning to userspace.
 *
 * @param t Thread whose values to set.
 * @param a First value.
 * @param b Second value.
 * @param c Third value.
 * @param d Fourth value.
 * @param e Fifth value.
 */
#define set_ret5(t, a, b, c, d, e) set_ret(t, 5, SYS_RET5(a, b, c, d, e))

/**
 * Set six sys_ret values to pass to thread when returning to userspace.
 *
 * @param t Thread whose sys_ret values to set.
 * @param a First value.
 * @param b Second value.
 * @param c Third value.
 * @param d Fourth value.
 * @param e Fifth value.
 * @param f Sixth value.
 */
#define set_ret6(t, a, b, c, d, e, f) \
	set_ret(t, 6, SYS_RET6(a, b, c, d, e, f))

/**
 * Set one argument to pass to userspace. Sets the ID to be zero, i.e. kernel.
 *
 * @param t Thread whose arguments to set.
 * @param s Status argument.
 */
#define set_args1(t, s) \
	set_ret2(t, s, 0)

/**
 * Set two arguments to pass to userspace. Sets the ID to be zero, i.e. kernel.
 *
 * @param t Thread whose arguments to set.
 * @param s Status argument.
 * @param a First generic argument.
 */
#define set_args2(t, s, a) \
	set_ret3(t, s, 0, a)

/**
 * Set three arguments to pass to userspace. Sets the ID to be zero, i.e. kernel.
 *
 * @param t Thread whose arguments to set.
 * @param s Status argument.
 * @param a First generic argument.
 * @param b Second generic argument.
 */
#define set_args3(t, s, a, b) \
	set_ret4(t, s, 0, a, b)

/**
 * Set four arguments to pass to userspace. Sets the ID to be zero, i.e. kernel.
 *
 * @param t Thread whose arguments to set.
 * @param s Status argument.
 * @param a First generic argument.
 * @param b Second generic argument.
 * @param c Third generic argument.
 */
#define set_args4(t, s, a, b, c) \
	set_ret5(t, s, 0, a, b, c)

/**
 * Set five arguments to pass to userspace. Sets the ID to be zero, i.e. kernel.
 *
 * @param t Thread whose arguments to set.
 * @param s Status argument.
 * @param a First generic argument.
 * @param b Second generic argument.
 * @param c Third generic argument.
 * @param d Fourth generic argument.
 */
#define set_args5(t, s, a, b, c, d) \
	set_ret6(t, s, 0, a, b, c, d)

/**
 * Set status to return from uapi function.
 * Essentially a beauty macro and slight micro-optimization, avoiding setting
 * registers to zero when not required.
 *
 * @param t Thread whose arguments to set.
 * @param s Status.
 */
#define return_args1(t, s) \
	{set_args1(t, s); return;}

/**
 * Set one argument and status to return from uapi function.
 *
 * @param t Thread whose arguments to set.
 * @param s Status.
 * @param a First argument.
 */
#define return_args2(t, s, a) \
	{set_args2(t, s, a); return;}

/**
 * Set two arguments and status to return from uapi function.
 *
 * @param t Thread whose arguments to set.
 * @param s Status argument.
 * @param a First argument.
 * @param b Second argument.
 */
#define return_args3(t, s, a, b) \
	{set_args3(t, s, a, b); return;}

/**
 * Set three arguments and status to return from uapi function.
 *
 * @param t Thread whose arguments to set.
 * @param s Status argument.
 * @param a First argument.
 * @param b Second argument.
 * @param c Third argument.
 */
#define return_args4(t, s, a, b, c) \
	{set_args4(t, s, a, b, c); return;}

/**
 * Set four arguments and status to return from uapi function.
 *
 * @param t Thread whose arguments to set.
 * @param s Status argument.
 * @param a First argument.
 * @param b Second argument.
 * @param c Third argument.
 * @param d Fourth argument.
 */
#define return_args5(t, s, a, b, c, d) \
	{set_args4(t, s, a, b, c, d); return;}

/**
 * Set all arguments and return from uapi function.
 *
 * @param t Thread whose arguments to set.
 * @param x Arguments to set.
 */
#define return_args(t, x) {set_ret((t), 6, (x)); return;}

#endif /* KMI_UAPI_H */
