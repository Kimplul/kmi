/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef ARCH_RISCV_TCB_H
#define ARCH_RISCV_TCB_H

/**
 * @file tcb.h
 * riscv64 definitions of arch-specific tcb data.
 */

/**
 * riscv-specific thread handling stuff.
 */
struct arch_tcbd {
	/** Extra scratch register. */
	long scratch;
};

#endif /* ARCH_RISCV_TCB_H */
