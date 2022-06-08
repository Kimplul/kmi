#ifndef APOS_SYSCALLS_H
#define APOS_SYSCALLS_H

/**
 * @file syscalls.h
 * Table of system calls.
 */

/** enum for now, possibly macros in the future once I get an approximate idea of
 * which syscalls are necessary etc. */
enum {
	/** @name Misc. */
	/** @{ */
	/** Noop, mainly for testing syscall subsystem and sanity checking. */
	SYS_NOOP,
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

	/** Send signal to thread. */
	SYS_SIGNAL,

	/** Switch running process. */
	SYS_SWAP,
	/** @} */

	/** @name Kernel management. */
	/** @{ */
	/** Configure system parameters (stack size etc.). */
	SYS_CONF,

	/** Shutdown, reboot, etc. */
	SYS_POWEROFF,
	/** @} */
};

/* function declarations should be somewhere else, this file could be used in
 * userspace applications as well */

#endif /* APOS_SYSCALLS_H */
