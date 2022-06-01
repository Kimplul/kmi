#ifndef APOS_POWER_H
#define APOS_POWER_H

/**
 * @file power.h
 * Power subsystem. Will hopefully eventually be used to restart and shutdown
 * host machines.
 */

#include <apos/types.h>
#include <apos/attrs.h>

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
