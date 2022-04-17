#ifndef APOS_RISCV_SBI_H
#define APOS_RISCV_SBI_H

#include <apos/types.h>

struct sbiret {
	long error;
	long value;
};

enum {
	SBI_SUCCESS = 0,
	SBI_ERR_FAILED = -1,
	SBI_ERR_NOT_SUPPORTED = -2,
	SBI_ERR_INVALID_PARAM = -3,
	SBI_ERR_DENIED = -4,
	SBI_ERR_INVALID_ADDRESS = -5,
	SBI_ERR_ALREADY_AVAILABLE = -6,
	SBI_ERR_ALREADY_STARTED = -7,
	SBI_ERR_ALREADY_STOPEED = -8,
};

/* TODO: query which extensions are available */
struct sbiret sbi_ecall(int ext, int fid, unsigned long arg0,
                        unsigned long arg1, unsigned long arg2,
                        unsigned long arg3, unsigned long arg4,
                        unsigned long arg5);

#define EID_TIME 0x54494D45
#define FID_SET_TIMER 0
static inline struct sbiret sbi_set_timer(uint64_t stime_value)
{
	/* TODO: read timebase from fdt, seems to be clocks/sec for accurate
	 * timers */
#if __riscv_xlen == 32
	return sbi_ecall(EID_TIME, FID_SET_TIMER, stime_value, stime_value >> 32, 0, 0, 0, 0);
#else
	return sbi_ecall(EID_TIME, FID_SET_TIMER, stime_value, 0, 0, 0, 0, 0);
#endif
}

#define EID_SRST 0x53525354
#define FID_RESET 0
static inline struct sbiret sbi_system_reset(uint32_t reset_type, uint32_t reset_reason)
{
	return sbi_ecall(EID_SRST, FID_RESET, reset_type, reset_reason, 0, 0, 0, 0);
}

#endif /* APOS_RISCV_SBI_H */
