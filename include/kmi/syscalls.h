/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_SYSCALLS_H
#define KMI_SYSCALLS_H

/**
 * @file syscalls.h
 * Table of system calls.
 */

/* pass VM_X etc. to userspace */
#if defined(__riscv)
# if __riscv_xlen == 64
#include "../../arch/riscv64/include/uapi.h"
# else
#include "../../arch/riscv32/include/uapi.h"
# endif
#endif

/** enum for now, possibly macros in the future once I get an approximate idea of
 * which syscalls are necessary etc. */
enum sys_code {
	/**
	 * @name Misc.
	 * Implementation in \ref dispatch.c instead of a separate file, as I
	 * consider them 'internal' and not intended for users.
	 */
	/** @{ */
	/** Noop, mainly for testing syscall subsystem and sanity checking. */
	SYS_NOOP,

	/**
	 * Put a single character to the serial lines.
	 * Do not rely on this actually working, as the serial drivers are only
	 * included on debugging kernels.
	 *
	 * \todo Should serial drivers be always included and debugging mode
	 * turned into whether \ref info is turned on or off or something?
	 */
	SYS_PUTCH,

	/** @} */

	/* @name Memory management. */
	/** @{ */
	/** Request memory from anywhere. */
	SYS_REQ_MEM,

	/** Request memory with physical address. */
	SYS_REQ_PMEM,

	/** Request memory at fixed virtual address. */
	SYS_REQ_FIXMEM,

	/** Request one page of memory, useful for stuff like virtio buffers */
	SYS_REQ_PAGE,

	/** Request shared memory. */
	SYS_REQ_SHAREDMEM,

	/** Request physical page from ram. */
	SYS_REF_SHAREDMEM,

	/** Free memory. */
	SYS_FREE_MEM,
	/** @} */

	/** @name Timers */
	/** @{ */
	/** Get accuracy of clock in Hertz. */
	SYS_TIMEBASE,

	/** Get current ticks. */
	SYS_TICKS,

	/** Request relative timer (number of ticks from now). */
	SYS_REQ_REL_TIMER,

	/** Request absolute timer (timepoint in ticks). */
	SYS_REQ_ABS_TIMER,

	/** Remove timer. */
	SYS_FREE_TIMER,
	/** @} */

	/** @name IPC. */
	/** @{ */
	/** Send IPC request as client. */
	SYS_IPC_REQ,

	/** Forward IPC request from client. */
	SYS_IPC_FWD,

	/** Tail call, i.e. 'do this for me and then return to whoever called
	 * me" */
	SYS_IPC_TAIL,

	/** Kick request handling to someone else. Forwards AND does a tailcall. */
	SYS_IPC_KICK,

	/** IPC response from server. */
	SYS_IPC_RESP,

	/** Notify thread, essentially interrupt or signal. */
	SYS_IPC_NOTIFY,
	/** @} */

	/** @name Process management. */
	/** @{ */
	/** Create new thread. */
	SYS_CREATE,

	/** Duplicate process. */
	SYS_FORK,

	/** Execute new binary in process space. */
	SYS_EXEC,

	/** Execute new binary in new process space. */
	SYS_SPAWN,

	/** Switch running process. */
	SYS_SWAP,

	/** @} */

	/** @name Kernel management. */
	/** @{ */
	/** Configure system parameters (stack size etc.). */
	SYS_SET_CONF,

	/** Get system parameters. */
	SYS_GET_CONF,

	/** Set capability of thread. */
	SYS_SET_CAP,

	/** Get capabilities of thread. */
	SYS_GET_CAP,

	/** Clear capability of thread. */
	SYS_CLEAR_CAP,

	/** Shutdown, reboot, etc. */
	SYS_POWEROFF,

	/** Set this core to sleep. */
	SYS_SLEEP,

	/** Request to handle IRQ. */
	SYS_IRQ_REQ,

	/** Retract IRQ. */
	SYS_FREE_IRQ,

	/** Request a notification handler. */
	SYS_SET_HANDLER,

	/** Request a thread exits. */
	SYS_EXIT,

	/** Detach a thread from its root process, becoming an orphant. */
	SYS_DETACH,

	/** @} */

	SYS_NUM,
};

/** Operation codes for reverse requests, i.e. where the kernel wants userspace
 * to do something. */
enum sys_user {
	/** Thread has received one or several notifications, please handle
	 * them. */
	SYS_USER_NOTIFY,

	/** Thread has been orphaned. */
	SYS_USER_ORPHANED,

	/** A new process has been spawned. Special case for init where each
	 * core starts with this, usually it is reserved for exec. */
	SYS_USER_SPAWNED,
};

/** Which notifications have arrived. */
enum notify_flag {
	/** A signal (\ref sys_ipc_notify()). */
	NOTIFY_SIGNAL = (1 << 0),

	/** A timer has expired. */
	NOTIFY_TIMER = (1 << 1),

	/** An interrupt request. */
	NOTIFY_IRQ = (1 << 2),

	/** Thread has become orphaned. */
	NOTIFY_ORPHANED = (1 << 3),
};

/** Types of powering off. Still unclear what difference there is between warm
 * and cold reboot. */
enum poweroff_type {
	/** Shut down. */
	SYS_SHUTDOWN,

	/** Cold or complete reboot. */
	SYS_COLD_REBOOT,

	/** Warm or partial reboot. */
	SYS_WARM_REBOOT
};

/* function declarations should be somewhere else, this file could be used in
 * userspace applications as well */

/**
 * Syscall argument type.
 *
 * \todo: Should this be arch specific? should be the size of an integer
 * register.
 */
typedef long sys_arg_t;

/** IDs for configuration parameters. */
enum conf_param {
	/**
	 * How large a regular thread stack is.
	 * \c R/W
	 */
	CONF_THREAD_STACK,

	/**
	 * How large an RPC stack element is. R/W. Note that this is not how
	 * large the RPC stack is in total, as that is hardcoded by the
	 * architecture, but how much of that stack one RPC call can take.
	 * \c R/W
	 */
	CONF_RPC_STACK,

	/**
	 * Current RAM usage.
	 * \c R
	 */
	CONF_RAM_USAGE,

	/**
	 * Total RAM size.
	 * \c R.
	 */
	CONF_RAM_SIZE,

	/**
	 * Size of page of some order.
	 * Uses `d0` to signify which order to request.
	 * \c R
	 */
	CONF_PAGE_SIZE,

	/**
	 * Maximum number of threads active at the same time.
	 * Can be used by userspace to build a pretty fast hashmaps of thread
	 * IDs to whatever, see \ref get_tcb().
	 * Guaranteed to be some power of 2.
	 * \c R
	 */
	CONF_MAX_THREADS,
};

/** Capabilities of process. */
enum sys_cap {
	/** Thread is allowed to set capabilities of other threads. */
	CAP_CAPS = (1 << 0),

	/** Thread is allowed to modify process statuses, create/exec/fork/etc. */
	CAP_PROC = (1 << 1),

	/** Thread is allowed to force notification in other thread. */
	CAP_NOTIFY = (1 << 2),

	/** Thread is allowed to shut down system. */
	CAP_POWER = (1 << 3),

	/** Thread is allowed to access configuration parameters. */
	CAP_CONF = (1 << 4),

	/** Thread is allowed to request to handle IRQs. */
	CAP_IRQ = (1 << 5),

	/** Thread is allowed to request notification handler. */
	CAP_SIGNAL = (1 << 6),

	/** Thread is allowed to request shared memory */
	CAP_SHARED = (1 << 7)
};

/**
 * Status codes.
 * Negative error codes are reserved for general usage, positive error codes are
 * allowed to be function-specific, although that's sort of difficult to keep
 * track of.
 */
enum sys_status {
	/** Permission error. */
	ERR_PERM = -10,
	/** Internal error, should probably halt */
	ERR_INT = -9,
	/** Something went wrong :/ */
	ERR_MISC = -8,
	/** Not initialized. */
	ERR_NOINIT = -7,
	/** Invalid value. */
	ERR_INVAL = -6,
	/** Already exists. */
	ERR_EXT = -5,
	/** Out of memory. */
	ERR_OOMEM = -4,
	/** Illegal address. */
	ERR_ADDR = -3,
	/** Wrong alignment. */
	ERR_ALIGN = -2,
	/** Not found. */
	ERR_NF = -1,
	/** OK. */
	OK = 0,
};

/**
 * Return structure of syscall.
 * \note Field names are generic, and can be used for whatever,
 * check documentation of whatever you're doing.
 * @todo should this be placed into syscalls.h?
 */
struct sys_ret {
	/** Status. */
	sys_arg_t s;

	/** Who responded */
	sys_arg_t id;

	/** First argument. */
	sys_arg_t a0;

	/** Second argument. */
	sys_arg_t a1;

	/** Third argument. */
	sys_arg_t a2;

	/** Fourth argument. */
	sys_arg_t a3;
};

#endif /* KMI_SYSCALLS_H */
