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

	/** Request physical page from ram. */
	SYS_REF_SHAREDMEM,

	/** Request memory with physical address. */
	SYS_REQ_PMEM,

	/** Request memory at fixed virtual address. */
	SYS_REQ_FIXMEM,

	/** Request shared memory. */
	SYS_REQ_SHAREDMEM,

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
	/** Inform kernel that process should be treated as server. */
	SYS_IPC_SERVER,

	/** Send IPC request as client. */
	SYS_IPC_REQ,   /* IPC request to server */

	/** Forward IPC request from client. */
	SYS_IPC_FWD,

	/** Kick request handling to someone else. */
	SYS_IPC_KICK,

	/** IPC response from server. */
	SYS_IPC_RESP,

	/** IPC return without visible side effects. */
	SYS_IPC_GHOST,

	/** Notify thread, essentially interrupt or signal. */
	SYS_NOTIFY,
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

	/** Kill thread. */
	SYS_KILL,

	/** Switch running process. */
	SYS_SWAP,

	/** @} */

	/** @name Kernel management. */
	/** @{ */
	/** Configure system parameters (stack size etc.). */
	SYS_CONF_SET,

	/** Get system parameters. */
	SYS_CONF_GET,

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

	/** Request a notification handler. */
	SYS_REQ_NOTIFICATION,

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
};

/** Which notifications have arrived. */
enum notify_flag {
	/** A signal (\ref sys_notify()). */
	NOTIFY_SIGNAL = (1 << 0),

	/** A timer has expired. */
	NOTIFY_TIMER = (1 << 1),

	/** An interrupt request. */
	NOTIFY_IRQ = (1 << 2),

	/** Thread has become orphaned. */
	NOTIFY_ORPHANED = (1 << 3),
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

#endif /* KMI_SYSCALLS_H */
