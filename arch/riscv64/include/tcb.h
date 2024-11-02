/* SPDX-License-Identifier: copyleft-next-0.3.1 */
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

	/** RPC stack page table leaf node. */
	struct vmem *rpc_leaf;

	/** O1 page that contains the whole stack */
	pm_t rpc_page;

	/** Index into \p rpc_leaf with the lowest accessed page so far in a
	 * certain context. */
	int rpc_idx;
};

#endif /* ARCH_RISCV_TCB_H */
