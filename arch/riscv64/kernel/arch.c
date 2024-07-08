/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file arch.c
 * Misc riscv specific stuff implementations.
 */

#include <kmi/assert.h>
#include <kmi/debug.h>
#include <kmi/power.h>
#include <kmi/bkl.h>
#include "csr.h"
#include "arch.h"

/** Array where indexing is done with CPU IDs, giving the corresponding hart ID. */
static id_t __cpuid_to_hartid[MAX_CPUS];

id_t cpuid_to_hartid(id_t cpu)
{
	return __cpuid_to_hartid[cpu];
}

void set_hartid(id_t cpu, id_t hartid)
{
	__cpuid_to_hartid[cpu] = hartid;
}

id_t hartid_to_cpuid(id_t hart)
{
	for (size_t i = 0; i < MAX_CPUS; ++i)
		if (cpuid_to_hartid(i) == hart)
			return i;

	/* put extra cores to sleep so they don't do anything bad */
	error("failed to match hart id %ld to cpu id\n", (long)hart);
	bkl_unlock();
	sleep();
	/* should never really be reached but eh */
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
