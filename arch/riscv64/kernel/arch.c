/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file arch.c
 * Misc riscv specific stuff implementations.
 */

#include <kmi/assert.h>
#include <kmi/debug.h>
#include "csr.h"
#include "arch.h"

id_t __cpuid_to_hartid[MAX_CPUS];

id_t hartid_to_cpuid(id_t hart)
{
	for (size_t i = 0; i < MAX_CPUS; ++i)
		if (cpuid_to_hartid(i) == hart)
			return i;

	error("failed to match hart id %ld to cpu id\n", (long)hart);
	/* default to zero, though this should maybe be a panic? */
	return 0;
}

pm_t branch_to_satp(struct vmem *branch, enum mm_mode mode)
{
	/* Sv57 && Sv64 in the future? */
	pm_t pn = (pm_t)(branch) >> page_shift();

	pm_t m = DEFAULT_Sv_MODE;

	switch (mode) {
	case Sv32: m = SATP_MODE_Sv32; break;
	case Sv39: m = SATP_MODE_Sv39; break;
	case Sv48: m = SATP_MODE_Sv48; break;
	default: bug("unknown satp mode\n"); break;
	}

	return m | pn;
}
