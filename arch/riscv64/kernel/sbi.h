/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_RISCV_SBI_H
#define KMI_RISCV_SBI_H

/**
 * @file sbi.h
 * riscv64-specific OpenSBI definitions.
 */

#include <kmi/types.h>

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
#define FID_TIME_SET_TIMER 0

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
	return sbi_ecall(EID_TIME, FID_TIME_SET_TIMER, stime_value,
	                 stime_value >> 32, 0, 0, 0, 0);
#else
	return sbi_ecall(EID_TIME, FID_TIME_SET_TIMER, stime_value,
	                 0, 0, 0, 0, 0);
#endif
}

/** System reset extension ID. */
#define EID_SRST 0x53525354

/** Function ID of sbi_system_reset(). */
#define FID_SRST_RESET 0

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
	return sbi_ecall(EID_SRST, FID_SRST_RESET, reset_type, reset_reason,
	                 0, 0, 0, 0);
}

/** Supervisor IPI extension ID. */
#define EID_sPI 0x735049

/** Supervisor IPI function ID. */
#define FID_sPI_IPI 0

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
	return sbi_ecall(EID_sPI, FID_sPI_IPI, hart_mask, hart_mask_base,
	                 0, 0, 0, 0);
}

/** Hart state management extension ID. */
#define EID_HSM 0x48534D

/** Hart start function ID. */
#define FID_HSM_START 0

/**
 * Start hart.
 *
 * @param hartid Hart to start.
 * @param start_addr Address to jump to in S mode.
 * @param opaque OS-specific argument passed to hart.
 * @return SBI call return. \see sbiret.
 */
static inline struct sbiret sbi_hart_start(unsigned long hartid,
                                           unsigned long start_addr,
                                           unsigned long opaque)
{
	return sbi_ecall(EID_HSM, FID_HSM_START, hartid, start_addr, opaque,
	                 0, 0, 0);
}

/** Hart start function ID. */
#define FID_HSM_STOP 1

/**
 * Stop current hart.
 *
 * @return SBI call return. \see sbiret.
 */
static inline struct sbiret sbi_hart_stop()
{
	return sbi_ecall(EID_HSM, FID_HSM_STOP, 0, 0, 0, 0, 0, 0);
}

/** Hart status function ID. */
#define FID_HSM_STATUS 2

/**
 * Get hart status.
 *
 * @param hartid Hart ID.
 * @return SBI call return. \see sbiret.
 */
static inline struct sbiret sbi_hart_status(unsigned long hartid)
{
	return sbi_ecall(EID_HSM, FID_HSM_STATUS, hartid,
	                 0, 0, 0, 0, 0);
}

/**
 * Status code \ref sbi_hart_status() returns for a started hart.
 * The only one we're currently interested in.
 */
#define SBI_HART_STARTED 0

#endif /* KMI_RISCV_SBI_H */
