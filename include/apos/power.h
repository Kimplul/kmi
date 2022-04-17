#ifndef APOS_POWER_H
#define APOS_POWER_H

#include <apos/types.h>
#include <apos/attrs.h>

enum poweroff_type {SHUTDOWN, COLD_REBOOT, WARM_REBOOT};
stat_t poweroff(enum poweroff_type type);

#endif
