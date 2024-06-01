/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file power.c
 * riscv64 implementation of arch-specific power handling.
 */

#include <kmi/power.h>
#include "sbi.h"

/**
 * Shutdown without any particular reason. Only reason kernel is likely to
 * need.
 */
#define SBI_NO_REASON 0

/** @name Shutdown types. */
/** @{ */

/** Regular shutdown. */
#define SBI_SHUTDOWN 0

/** Cold reboot. */
#define SBI_COLD_REBOOT 1

/** Warm reboot. */
#define SBI_WARM_REBOOT 2

/** @} */

stat_t poweroff(enum poweroff_type type)
{
	/** \todo this only shuts down the cpu itself, but may leave the SOC
	 * active. Should read from fdt poweroff and syscon-poweroff etc */
	switch (type) {
	case SHUTDOWN:
		sbi_system_reset(SBI_SHUTDOWN, SBI_NO_REASON);
		return ERR_MISC;

	case COLD_REBOOT:
		sbi_system_reset(SBI_COLD_REBOOT, SBI_NO_REASON);
		return ERR_MISC;

	case WARM_REBOOT:
		sbi_system_reset(SBI_WARM_REBOOT, SBI_NO_REASON);
		return ERR_MISC;
	};

	return ERR_INVAL;
}

stat_t sleep()
{
	sbi_hart_stop();
	return ERR_MISC;
}
