/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2024, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file except.c
 * riscv64 exception handling.
 */

#include <kmi/types.h>
#include <kmi/panic.h>
#include <kmi/debug.h>
#include <kmi/vmem.h>
#include <kmi/bkl.h>

/**
 * Handle exception. Intended to be called from entry.S.
 *
 * @param pc Where exception occurred.
 * @param addr What address caused the exception, if any.
 * @param id ID of exception, see riscv spec.
 */
void riscv_handle_exception(void *pc, void *addr, unsigned long id)
{
	switch (id) {
	case 12:
	case 13:
	case 15: {
		bkl_lock();
		handle_pagefault((vm_t)addr);
		bkl_unlock();
		break;
	}

	default:
		unhandled_panic(pc, addr, id);
	}
}
