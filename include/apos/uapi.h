/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_UAPI_H
#define APOS_UAPI_H

/**
 * @file uapi.h
 * Userspace api, syscall declarations.
 */

#include <apos/syscalls.h>
#include <apos/vmem.h>

/**
 * Syscall function type.
 * Let's start with four arguments and see where that goes
 */
typedef struct sys_ret (*sys_t)(long, long, long, long);

/**
 * Syscall argument type.
 *
 * \todo: Should this be arch specific? should be the size of an integer
 * register.
 */
typedef long sys_arg_t;

/**
 * Return structure of syscall.
 * \note Field names are generic, and can be used for two values or two error
 * codes, check documentation of whatever you're doing.
 */
struct sys_ret {
	/** Error code of syscall. */
	sys_arg_t err;

	/** Value returned by syscall. */
	sys_arg_t val;
};

/**
 * Helper macro for declaring syscalls with zero arguments.
 *
 * The idea is that all syscalls externally have the same amount of arguments,
 * but then by using some helper macros (see below) we can make actually
 * implementing the syscall more intuitive and less noisy.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DECLARE0(name)                              \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, \
	                          sys_arg_t c,              \
	                          sys_arg_t d);

/**
 * Helper macro for declaring syscalls with one argument.
 *
 * @param name Name of syscall.
 * @param a Name of argument.
 */
#define SYSCALL_DECLARE1(name, a)                                        \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d);

/**
 * Helper macro for declaring syscalls with two arguments.
 *
 * @param name Name of syscall.
 * @param a Name of first argument.
 * @param b Name of second argument.
 */
#define SYSCALL_DECLARE2(name, a, b)                                     \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d);

/**
 * Helper macro for declaring syscalls with three arguments.
 *
 * @param name Name of syscall.
 * @param a Name of first argument.
 * @param b Name of second argument.
 * @param c Name of third argument.
 */
#define SYSCALL_DECLARE3(name, a, b, c)                                  \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d);

/**
 * Helper macro for declaring syscalls with four arguments.
 *
 * @param name Name of syscall.
 * @param a Name of first argument.
 * @param b Name of second argument.
 * @param c Name of third argument.
 * @param d Name of fourth argument.
 */
#define SYSCALL_DECLARE4(name, a, b, c, d)                               \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d);

/**
 * Helper macro for defining syscall with zero arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE0(name)                                            \
	static inline struct sys_ret __##name();                         \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d)                           \
	{                                                                \
		UNUSED(a);                                               \
		UNUSED(b);                                               \
		UNUSED(c);                                               \
		UNUSED(d);                                               \
		return __##name();                                       \
	}                                                                \
	static struct sys_ret __##name

/**
 * Helper macro for defining syscall with one argument.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE1(name)                                            \
	static inline struct sys_ret __##name(sys_arg_t);                \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d)                           \
	{                                                                \
		UNUSED(b);                                               \
		UNUSED(c);                                               \
		UNUSED(d);                                               \
		return __##name(a);                                      \
	}                                                                \
	static inline struct sys_ret __##name

/**
 * Helper macro for defining syscall with two arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE2(name)                                            \
	static inline struct sys_ret __##name(sys_arg_t, sys_arg_t);     \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d)                           \
	{                                                                \
		UNUSED(c);                                               \
		UNUSED(d);                                               \
		return __##name(a, b);                                   \
	}                                                                \
	static inline struct sys_ret __##name

/**
 * Helper macro for defining syscall with three arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE3(name)                                            \
	static inline struct sys_ret __##name(sys_arg_t, sys_arg_t,      \
	                                      sys_arg_t);                \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d)                           \
	{                                                                \
		UNUSED(d);                                               \
		return __##name(a, b, c);                                \
	}                                                                \
	static inline struct sys_ret __##name

/**
 * Helper macro for defining syscall with four arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE4(name)                                                  \
	static inline struct sys_ret __##name(sys_arg_t, sys_arg_t, sys_arg_t, \
	                                      sys_arg_t);                      \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c,       \
	                          sys_arg_t d)                                 \
	{                                                                      \
		return __##name(a, b, c, d);                                   \
	}                                                                      \
	static inline struct sys_ret __##name

/**
 * Noop syscall.
 *
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and 0.
 */
SYSCALL_DECLARE0(noop);

/* @name Memory handling syscalls. */
/** @{ */
/**
 * Request memory syscall.
 *
 * Allocates at least the specified size of allocation to current effective
 * process.
 *
 * @param size Size of allocation.
 * @param flags Flags of allocation.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and start of memory allocation.
 */
SYSCALL_DECLARE2(req_mem, size, flags);

/**
 * Request physical memory syscall.
 *
 * Allocates at least the specified size of allocation which includes the
 * physical start address somewhere in the allocation to the current effective
 * process.
 *
 * @param paddr Start of physical allocation.
 * @param size Size of physical allocation.
 * @param flags Flags of physical allocation.
 * @param d Unused.
 * @return \ref OK and start of memory allocation.
 */
SYSCALL_DECLARE3(req_pmem, paddr, size, flags);

/**
 * Request fixed memory syscall.
 *
 * Allocates at least the specified size of allocation which includes the start
 * address of allocation to the current effective process.
 *
 * @param start Start of allocation.
 * @param size Size of allocation.
 * @param flags Flags of allocation.
 * @param d Unused.
 * @return \ref OK and start of memory allocation.
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
 * @param size Size of allocation.
 * @param flags Flags of allocation.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and start of memory allocation.
 */
SYSCALL_DECLARE2(req_sharedmem, size, flags);

/**
 * Reference shared memory syscall.
 *
 * \see sys_req_sharedmem().
 *
 * @param tid Thread ID of owner of shared memory.
 * @param va Start address of shared memory.
 * @param flags Flags to use for reference.
 * @param d Unused.
 * @return \ref OK and start of shared memory.
 */
SYSCALL_DECLARE3(ref_sharedmem, tid, va, flags);

/**
 * Free memory syscall.
 *
 * Frees the memory region pointed to.
 *
 * @param start Start of memory.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and 0.
 */
SYSCALL_DECLARE1(free_mem, start);
/** @} */

/** @name Timer syscalls. */
/** @{ */
/**
 * Get timer accuracy in Hertz syscall.
 *
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and frequency.
 * \todo Should this be some kind of config request instead of a separate
 * syscall?
 */
SYSCALL_DECLARE0(timebase);

/**
 * Request relative timer syscall.
 *
 * Request timer that triggers a number of ticks in the future.
 *
 * @param ticks Number of ticks from now.
 * @param mult Number of times to trigger.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and ID of timer.
 */
SYSCALL_DECLARE2(req_rel_timer, ticks, mult);

/**
 * Request absolute timer syscall.
 *
 * Request timer that triggers at some absolute timepoint.
 *
 * @param ticks Timepoint.
 * @param mult Multiplier.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and ID of timer.
 * \todo Check repeat value.
 */
SYSCALL_DECLARE2(req_abs_timer, ticks, mult);

/**
 * Free timer syscall.
 *
 * @param cid ID of timer to free.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and 0.
 */
SYSCALL_DECLARE1(free_timer, cid);
/** @} */

/** @name IPC syscalls. */
/** @{ */
/**
 * Report process status as server syscall.
 *
 * @param callback Callback to request handler.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and 0.
 */
SYSCALL_DECLARE1(ipc_server, callback);

/**
 * Request syscall.
 *
 * @param pid Request target process.
 * @param d0 First request argument.
 * @param d1 Second request argument.
 * @param d Unused.
 * @return \c d0 and \c d1.
 */
SYSCALL_DECLARE3(ipc_req, pid, d0, d1);

/**
 * Forwarding syscall.
 *
 * In a server request handler, do a request to some other server on behalf of
 * whoever called us up.
 *
 * @param pid Forwarding target process.
 * @param d0 First forwarding argument.
 * @param d1 Second forwarding argument.
 * @param d Unused.
 * @return \c d0 and \c d1.
 */
SYSCALL_DECLARE3(ipc_fwd, pid, d0, d1);

/**
 * Response syscall.
 *
 * @param d0 First response argument.
 * @param d1 Second response argument.
 * @param c Unused.
 * @param d Unused.
 * @return \c d0 and \c d1.
 */
SYSCALL_DECLARE2(ipc_resp, d0, d1);
/** @} */

/** @name Process handling syscalls. */
/** @{ */
/**
 * Create thread syscall.
 *
 * Creates thread in current effective process context.
 *
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and 0.
 * \todo Should this take stack size etc?
 */
SYSCALL_DECLARE0(create);

/**
 * Fork process syscall.
 *
 * Forks a process, much like in *nix systems.
 *
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and 0.
 */
SYSCALL_DECLARE0(fork);

/**
 * Execute binary syscall.
 *
 * Executes a new binary in existing process space.
 *
 * @param bin Address of binary.
 * @param interp Optional address of interpreter.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and 0.
 * \todo Check other return codes.
 */
SYSCALL_DECLARE2(exec, bin, interp);

/**
 * Signal process syscall.
 *
 * @param tid Thread ID to signal.
 * @param signal Signal to send.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and 0.
 */
SYSCALL_DECLARE2(signal, tid, signal);

/**
 * Swap syscall.
 *
 * Swap currently running thread.
 *
 * @param tid Thread to swap to.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and 0.
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
 * @param param Parameter to set.
 * @param val Value to set parameter to.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and 0.
 */
SYSCALL_DECLARE2(conf_set, param, val);

/**
 * Get configuration syscall.
 *
 * Get some runtime parameter.
 *
 * @param param Parameter to get.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @return \ref OK and 0.
 */
SYSCALL_DECLARE1(conf_get, param);

/**
 * Power off syscall.
 *
 * Either shut down or reboot system.
 *
 * @param type Type of shutdown. \see poweroff_type.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @return Shouldn't return at all.
 */
SYSCALL_DECLARE1(poweroff, type);
/** @} */

/**
 * Dispatch to correct syscall handler.
 *
 * @param syscall Syscall number.
 * @param a Syscall argument 0.
 * @param b Syscall argument 1.
 * @param c Syscall argument 2.
 * @param d Syscall argument 3.
 * @return Whatever the specified syscall returns.
 */
struct sys_ret syscall_dispatch(sys_arg_t syscall, sys_arg_t a, sys_arg_t b,
                                sys_arg_t c, sys_arg_t d);

/** \todo Should I add variable names as well, to make the documentation a bit
 * more readable? */
#endif /* APOS_UAPI_H */
