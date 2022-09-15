/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_SYSCALLS_H
#define APOS_SYSCALLS_H

/**
 * @file syscalls.h
 * Table of system calls.
 */

/** enum for now, possibly macros in the future once I get an approximate idea of
 * which syscalls are necessary etc. */
enum {
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

	/** IPC response from server. */
	SYS_IPC_RESP,
	/** @} */

	/** @name Process management. */
	/** @{ */
	/** Create new thread. */
	SYS_CREATE,

	/** Duplicate process. */
	SYS_FORK,

	/** Execute new binary in process space. */
	SYS_EXEC,

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

	/** Shutdown, reboot, etc. */
	SYS_POWEROFF,
	/** @} */
};

/* function declarations should be somewhere else, this file could be used in
 * userspace applications as well */

#endif /* APOS_SYSCALLS_H */
