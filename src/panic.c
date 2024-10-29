/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file panic.c
 * Kernel panic handler implementation.
 */

#include <kmi/syscalls.h>
#include <kmi/power.h>
#include <kmi/debug.h>
#include <kmi/tcb.h>

void kernel_panic(void *pc, void *addr, long cause)
{
	/* could be useful to print out register values as well? */
	error("kernel paniced at pc: %p with address %p and cause %lx\n",
	      pc, addr, cause);

	info("attempting to reboot\n");

	poweroff(SYS_COLD_REBOOT);

	/* spin if poweroff failed for some reason */
	error("reboot failed, spinning in place\n");
	while (1);
}

void unhandled_panic(void *pc, void *addr, long cause)
{
	/* could be useful to print out register values as well? */
	error("thread %d unhandled panic at pc: %p with address %p and cause %lx\n",
	      cur_tcb()->cpu_id, pc, addr, cause);

	info("attempting to reboot\n");

	poweroff(SYS_COLD_REBOOT);

	/* spin if poweroff failed for some reason */
	error("reboot failed, spinning in place\n");
	while (1);
}
