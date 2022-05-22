/**
 * @file power.c
 * riscv64 implementation of arch-specific power handling.
 */

#include <apos/power.h>
#include <sbi.h>

/* shutdown reason */
#define SBI_NO_REASON   0

/* shutdown type */
#define SBI_SHUTDOWN    0
#define SBI_COLD_REBOOT 1
#define SBI_WARM_REBOOT 2

stat_t poweroff(enum poweroff_type type)
{
	/* TODO: this only shuts down the cpu itself, but may leave the SOC
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
