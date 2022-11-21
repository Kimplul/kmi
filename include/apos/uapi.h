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
 * Let's start with five arguments and see where that goes
 */
typedef struct sys_ret (*sys_t)(struct tcb *t, long, long, long, long, long);

/**
 * Syscall argument type.
 *
 * \todo: Should this be arch specific? should be the size of an integer
 * register.
 */
typedef long sys_arg_t;

/**
 * Return structure of syscall.
 * \note Field names are generic, and can be used for whatever,
 * check documentation of whatever you're doing.
 * @todo should this be placed into syscalls.h?
 */
struct sys_ret {
	/** Status. */
	sys_arg_t s;

	/** First argument. */
	sys_arg_t ar0;

	/** Second argument. */
	sys_arg_t ar1;

	/** Third argument. */
	sys_arg_t ar2;

	/** Fourth argument. */
	sys_arg_t ar3;

	/** Fifth argument. */
	sys_arg_t ar4;
};

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
#define SYSCALL_DECLARE0(name)                 \
	struct sys_ret sys_##name(struct tcb *t, \
				  sys_arg_t a, \
	                          sys_arg_t b, \
	                          sys_arg_t c, \
	                          sys_arg_t d, \
	                          sys_arg_t e);

/**
 * Helper macro for declaring syscalls with one argument.
 *
 * @param name Name of syscall.
 * @param a Name of argument.
 */
#define SYSCALL_DECLARE1(name, a)                                        \
	struct sys_ret sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d, sys_arg_t e);

/**
 * Helper macro for declaring syscalls with two arguments.
 *
 * @param name Name of syscall.
 * @param a Name of first argument.
 * @param b Name of second argument.
 */
#define SYSCALL_DECLARE2(name, a, b)                                     \
	struct sys_ret sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d, sys_arg_t e);

/**
 * Helper macro for declaring syscalls with three arguments.
 *
 * @param name Name of syscall.
 * @param a Name of first argument.
 * @param b Name of second argument.
 * @param c Name of third argument.
 */
#define SYSCALL_DECLARE3(name, a, b, c)                                  \
	struct sys_ret sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, sys_arg_t c, \
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
#define SYSCALL_DECLARE4(name, a, b, c, d)                               \
	struct sys_ret sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, sys_arg_t c, \
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
#define SYSCALL_DECLARE5(name, a, b, c, d, e)                            \
	struct sys_ret sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d, sys_arg_t e);

/**
 * Helper macro for defining syscall with zero arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE0(name)                                            \
	static inline struct sys_ret __##name(struct tcb *t);                         \
	struct sys_ret sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d, sys_arg_t e)              \
	{                                                                \
		UNUSED(a);                                               \
		UNUSED(b);                                               \
		UNUSED(c);                                               \
		UNUSED(d);                                               \
		UNUSED(e);                                               \
		return __##name(t);                                       \
	}                                                                \
	static struct sys_ret __##name

/**
 * Helper macro for defining syscall with one argument.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE1(name)                                            \
	static inline struct sys_ret __##name(struct tcb *, sys_arg_t);                \
	struct sys_ret sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d, sys_arg_t e)              \
	{                                                                \
		UNUSED(b);                                               \
		UNUSED(c);                                               \
		UNUSED(d);                                               \
		UNUSED(e);                                               \
		return __##name(t, a);                                      \
	}                                                                \
	static inline struct sys_ret __##name

/**
 * Helper macro for defining syscall with two arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE2(name)                                            \
	static inline struct sys_ret __##name(struct tcb *, sys_arg_t, sys_arg_t);     \
	struct sys_ret sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d, sys_arg_t e)              \
	{                                                                \
		UNUSED(c);                                               \
		UNUSED(d);                                               \
		UNUSED(e);                                               \
		return __##name(t, a, b);                                   \
	}                                                                \
	static inline struct sys_ret __##name

/**
 * Helper macro for defining syscall with three arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE3(name)                                            \
	static inline struct sys_ret __##name(struct tcb *, sys_arg_t, sys_arg_t,      \
	                                      sys_arg_t);                \
	struct sys_ret sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, sys_arg_t c, \
	                          sys_arg_t d, sys_arg_t e)              \
	{                                                                \
		UNUSED(d);                                               \
		UNUSED(e);                                               \
		return __##name(t, a, b, c);                                \
	}                                                                \
	static inline struct sys_ret __##name

/**
 * Helper macro for defining syscall with four arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE4(name)                                                  \
	static inline struct sys_ret __##name(struct tcb *, sys_arg_t, sys_arg_t, sys_arg_t, \
	                                      sys_arg_t);                      \
	struct sys_ret sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, sys_arg_t c,       \
	                          sys_arg_t d, sys_arg_t e)                    \
	{                                                                      \
		UNUSED(e);                                                     \
		return __##name(t, a, b, c, d);                                   \
	}                                                                      \
	static inline struct sys_ret __##name

/**
 * Helper macro for defining syscall with five arguments.
 *
 * @param name Name of syscall.
 */
#define SYSCALL_DEFINE5(name)                                                  \
	static inline struct sys_ret __##name(struct tcb *, sys_arg_t, sys_arg_t, sys_arg_t, \
	                                      sys_arg_t, sys_arg_t);           \
	struct sys_ret sys_##name(struct tcb *t, sys_arg_t a, sys_arg_t b, sys_arg_t c,       \
	                          sys_arg_t d, sys_arg_t e)                    \
	{                                                                      \
		return __##name(t, a, b, c, d, e);                                \
	}                                                                      \
	static inline struct sys_ret __##name

/** @name Misc syscalls. */
/** @{ */

/**
 * Noop syscall.
 *
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 * @return \ref OK and 0.
 */
SYSCALL_DECLARE0(noop);

/**
 * Putch syscall.
 *
 * @param ch Character to put.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 * @return \ref OK and 0.
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
 * @param size Size of allocation.
 * @param flags Flags of allocation.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
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
 * @param e Unused.
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
 * @param e Unused.
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
 * @param e Unused.
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
 * @param e Unused.
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
 * @param e Unused.
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
 * @param e Unused.
 * @return \ref OK and frequency.
 * \todo Should this be some kind of config request instead of a separate
 * syscall?
 */
SYSCALL_DECLARE0(timebase);

/**
 * Get current ticks.
 *
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 * @return Current ticks.
 */
SYSCALL_DECLARE0(ticks);

/**
 * Request relative timer syscall.
 *
 * Request timer that triggers a number of ticks in the future.
 *
 * @param ticks Number of ticks from now.
 * @param mult Number of times to trigger.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
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
 * @param e Unused.
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
 * @param e Unused.
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
 * @param e Unused.
 * @return \ref OK and 0.
 */
SYSCALL_DECLARE1(ipc_server, callback);

/**
 * Request syscall.
 *
 * @param pid Request target process.
 * @param d0 First request argument.
 * @param d1 Second request argument.
 * @param d2 Third forwarding argument.
 * @param d3 Fourth forwarding argument.
 * @return \c d0 and \c d1.
 */
SYSCALL_DECLARE5(ipc_req, pid, d0, d1, d2, d3);

/**
 * Forwarding syscall.
 *
 * In a server request handler, do a request to some other server on behalf of
 * whoever called us up.
 *
 * @param pid Forwarding target process.
 * @param d0 First forwarding argument.
 * @param d1 Second forwarding argument.
 * @param d2 Third forwarding argument.
 * @param d3 Fourth forwarding argument.
 * @return \c d0 and \c d1.
 */
SYSCALL_DECLARE5(ipc_fwd, pid, d0, d1, d2, d3);

/**
 * Response syscall.
 *
 * @param d0 First response argument.
 * @param d1 Second response argument.
 * @param d2 Third response argument.
 * @param d3 Fourth response argument.
 * @param e Unused.
 * @return \c d0 and \c d1.
 */
SYSCALL_DECLARE4(ipc_resp, d0, d1, d2, d3);

/**
 * Notify thread syscall.
 *
 * @param tid Thread ID to notify.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 * @return \ref OK and 0.
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
 * @param func Function to call on startup.
 * @param d0 Argument 0.
 * @param d1 Argument 1.
 * @param d2 Argument 2.
 * @param d3 Argument 3.
 *
 * @return \ref OK and 0.
 * \todo Should this take stack size etc?
 */
SYSCALL_DECLARE5(create, func, d0, d1, d2, d3);

/**
 * Fork process syscall.
 *
 * Forks a process, much like in *nix systems.
 *
 * @param a Unused.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
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
 * @param e Unused.
 * @return \ref OK and 0.
 * \todo Check other return codes.
 */
SYSCALL_DECLARE2(exec, bin, interp);

/**
 * Spawn binary syscall. Try to prefer over unixy fork/exec.
 *
 * Executes a new binary in new process space.
 *
 * @param bin Address of binary.
 * @param interp Optional address of interpreter.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 * @return \ref OK and 0.
 * \todo Check other return codes.
 */
SYSCALL_DECLARE2(spawn, bin, interp);

/**
 * Kill syscall.
 *
 * @param tid Thread ID to kill. 0 if self.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 * @return No.
 */
SYSCALL_DECLARE1(kill, tid);

/**
 * Swap syscall.
 *
 * Swap currently running thread.
 *
 * @param tid Thread to swap to.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
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
 * @param e Unused.
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
 * @param e Unused.
 * @return \ref OK.
 */
SYSCALL_DECLARE1(conf_get, param);

/**
 * Set capabilities.
 *
 * @param tid Thread ID whose capabilities to set.
 * @param off Offset of capability, multiple of \c bits(cap).
 * @param caps Mask of capabilities to set.
 * @param d Unused.
 * @param e Unused.
 * @return \ref OK on success, \ref ERR_INVAL on invalid input.
 */
SYSCALL_DECLARE3(set_cap, tid, off, caps);

/**
 * Get capabilities.
 *
 * @param tid Thread ID whose capabilities to get.
 * @param off Offset of capability, multiple of \c bits(cap).
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 * @return \ref OK, capabilities.
 */
SYSCALL_DECLARE2(get_cap, tid, off);

/**
 * Clear capabilities.
 *
 * @param tid Thread ID whose capabilities to clear.
 * @param off Offset of capability, multiple of \c bits(cap).
 * @param cap Mask of capabilities to clear.
 * @param d Unused.
 * @param e Unused.
 * @return \ref OK.
 */
SYSCALL_DECLARE3(clear_cap, tid, off, cap);

/**
 * Power off syscall.
 *
 * Either shut down or reboot system.
 *
 * @param type Type of shutdown. \see poweroff_type.
 * @param b Unused.
 * @param c Unused.
 * @param d Unused.
 * @param e Unused.
 * @return Shouldn't return at all.
 */
SYSCALL_DECLARE1(poweroff, type);
/** @} */

/**
 * Dispatch to correct syscall handler.
 *
 * @param t Thread to do syscall on.
 * @param syscall Syscall number.
 * @param a Syscall argument 0.
 * @param b Syscall argument 1.
 * @param c Syscall argument 2.
 * @param d Syscall argument 3.
 * @param e Syscall argument 4.
 * @return Whatever the specified syscall returns.
 */
struct sys_ret handle_syscall(struct tcb *t,
                              sys_arg_t syscall, sys_arg_t a, sys_arg_t b,
                              sys_arg_t c, sys_arg_t d, sys_arg_t e);

/** \todo Should I add variable names as well, to make the documentation a bit
 * more readable? */
#endif /* APOS_UAPI_H */
