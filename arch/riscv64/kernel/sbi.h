/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_RISCV_SBI_H
#define APOS_RISCV_SBI_H

/**
 * @file sbi.h
 * riscv64-specific OpenSBI definitions.
 */

#include <apos/types.h>

/** Return structure of SBI calls. */
struct sbiret {
	/** Error code. \see sbi_ecodes */
	long error;
	/** Value. */
	long value;
};

/** SBI error codes. */
enum sbi_ecodes {
	/** Succesful call. */
	SBI_SUCCESS = 0,
	/** Unsuccesful call. */
	SBI_ERR_FAILED = -1,
	/** Not supported. */
	SBI_ERR_NOT_SUPPORTED = -2,
	/** Invalid parameter. */
	SBI_ERR_INVALID_PARAM = -3,
	/** Access denied. */
	SBI_ERR_DENIED = -4,
	/** Invalid address. */
	SBI_ERR_INVALID_ADDRESS = -5,
	/** Already available. */
	SBI_ERR_ALREADY_AVAILABLE = -6,
	/** Already started. */
	SBI_ERR_ALREADY_STARTED = -7,
	/** Already stopped. */
	SBI_ERR_ALREADY_STOPPED = -8,
};

/**
 * Do an SBI call.
 *
 * @param ext \c EXT field of call.
 * @param fid \c FID field of call.
 * @param arg0 Argument 0.
 * @param arg1 Argument 1.
 * @param arg2 Argument 2.
 * @param arg3 Argument 3.
 * @param arg4 Argument 4.
 * @param arg5 Argument 5.
 * @return SBI call return. \see sbiret.
 * \todo Query which extensions are available.
 */
struct sbiret sbi_ecall(int ext, int fid, unsigned long arg0,
                        unsigned long arg1, unsigned long arg2,
                        unsigned long arg3, unsigned long arg4,
                        unsigned long arg5);

/** Timer extension ID. */
#define EID_TIME 0x54494D45

/** Function ID of sbi_set_timer(). */
#define FID_SET_TIMER 0

/**
 * Start timer. IRQs have to be enabled for the timer to trigger.
 *
 * @param stime_value Absolute timepoint in ticks.
 * @return SBI call return. \see sbiret.
 * \todo Read timebase from fdt, seems to be clocks/sec for accurate timers.
 */
static inline struct sbiret sbi_set_timer(uint64_t stime_value)
{
#if defined(riscv32)
	return sbi_ecall(EID_TIME, FID_SET_TIMER, stime_value,
	                 stime_value >> 32, 0, 0, 0, 0);
#else
	return sbi_ecall(EID_TIME, FID_SET_TIMER, stime_value, 0, 0, 0, 0, 0);
#endif
}

/** System reset extension ID. */
#define EID_SRST 0x53525354

/** Function ID of sbi_system_reset(). */
#define FID_RESET 0

/**
 * Reset system.
 *
 * @param reset_type Type of reset. \see SBI_SHUTDOWN, SBI_COLD_REBOOT,
 * SBI_WARM_REBOOT.
 * @param reset_reason Reason for reset. Optional, probably won't be used by the
 * kernel.
 * @return SBI call return \see sbiret.
 * \todo Should \ref SBI_SHUTDOWN etc. be defined in this file instead?
 */
static inline struct sbiret sbi_system_reset(uint32_t reset_type,
                                             uint32_t reset_reason)
{
	return sbi_ecall(EID_SRST, FID_RESET, reset_type, reset_reason, 0, 0, 0,
	                 0);
}

/** Supervisor IPI extension ID. */
#define EID_sPI 0x735049

/** Supervisor IPI function ID. */
#define FID_IPI 0

/**
 * Send IPI.
 *
 * @param hart_mask Bitmap of harts to send IPI to.
 * @param hart_mask_base Base of \p hart_mask.
 * @return SBI call return. \see sbiret.
 */
static inline struct sbiret sbi_send_ipi(unsigned long hart_mask,
                                         unsigned long hart_mask_base)
{
	return sbi_ecall(EID_sPI, FID_IPI, hart_mask, hart_mask_base, 0, 0, 0,
	                 0);
}

#endif /* APOS_RISCV_SBI_H */
