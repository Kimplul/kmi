/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_POWER_H
#define KMI_POWER_H

/**
 * @file power.h
 * Power subsystem. Will hopefully eventually be used to restart and shutdown
 * host machines.
 */

#include <kmi/types.h>
#include <kmi/attrs.h>

/** Types of powering off. Still unclear what difference there is between warm
 * and cold reboot. */
enum poweroff_type {
	/** Shut down. */
	SHUTDOWN,

	/** Cold or complete reboot. */
	COLD_REBOOT,

	/** Warm or partial reboot. */
	WARM_REBOOT
};

/**
 * Power off the system.
 *
 * @param type Type of powering off. \see poweroff_type.
 * @return Nothing on success (system shuts down), \ref ERR_INVAL on invalid
 * poweroff type or ERR_MISC if studown was not succesful.
 */
stat_t poweroff(enum poweroff_type type);

#endif
