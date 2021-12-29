#ifndef APOS_SYSCALLS_H
#define APOS_SYSCALLS_H

/* enum for now, possibly macros in the future once I get an approximate idea of
 * which syscalls are necessary etc. */
enum {
	/* memory management */
	SYS_REQ_MEM, /* request memory from anywhere */
	SYS_REQ_PMEM, /* request physical address */
	SYS_REQ_FIXMEM, /* request memory at fixed address */
	SYS_FREE_MEM, /* free memory */

	/* IPC */
	/* I really should try to find my notes about the server/client
	 * structure of the OS, but these following syscalls are probably
	 * required */
	SYS_IPC_SERVER, /* inform kernel that process should be treated as a server */
	SYS_IPC_REQ, /* IPC request to server */
	SYS_IPC_RESP, /* IPC response from server */

	/* process management */
	SYS_FORK, /* duplicate process */
	SYS_EXEC, /* execute new binary in process space */
	SYS_SIGNAL, /* send signal to process (kill etc.) */
	SYS_SWITCH, /* switch running process */
	SYS_SYNC, /* used by init process to synchronize running processes, maybe not used */

	/* kernel management */
	SYS_CONF, /* config system parameters (stack size etc.) */
	SYS_POWEROFF, /* shutdown/reboot etc. */
}

/* function declarations should be somewhere else, this file could be used in
 * userspace applications as well */

#endif /* APOS_SYSCALLS_H */