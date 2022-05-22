#ifndef APOS_POWER_H
#define APOS_POWER_H

/**
 * @file power.h
 * Power subsystem. Will hopefully eventually be used to restart and shutdown
 * host machines.
 */

#include <apos/types.h>
#include <apos/attrs.h>

enum poweroff_type { SHUTDOWN, COLD_REBOOT, WARM_REBOOT };
stat_t poweroff(enum poweroff_type type);

#endif
