#ifndef APOS_DEBUG_H
#define APOS_DEBUG_H

#include <apos/attrs.h>
#include <apos/pmem.h>

#ifdef DEBUG
enum serial_dev {
	/* only the NS16550A and compatible at the moment */
	NS16550A,
};

struct dbg_info {
	pm_t dbg_ptr;
	enum serial_dev dev;
};

void __fmt(1, 2) dbg(const char *fmt, ...);
void setup_dbg(pm_t pt, enum serial_dev dev);
struct dbg_info dbg_from_fdt(void *fdt);

#define COMMON_FORMAT "[%s] %s:%d\n\t"
#define COMMON_ARGS(s) s, __FILE__, __LINE__
#define bug(fmt, ...) dbg(COMMON_FORMAT fmt, COMMON_ARGS("BUG"), __VA_ARGS__)
#define warn(fmt, ...) dbg(COMMON_FORMAT fmt, COMMON_ARGS("WARN"), __VA_ARGS__)
#define info(fmt, ...) dbg(COMMON_FORMAT fmt, COMMON_ARGS("INFO"), __VA_ARGS__)
#define error(fmt, ...) dbg(COMMON_FORMAT fmt, COMMON_ARGS("ERROR"), __VA_ARGS__)

#else

#define dbg(...)
#define dbg_init(...)
#define dbg_from_fdt(...)

#define bug(...)
#define warn(...)
#define info(...)
#define error(...)

#endif /* DEBUG */

#endif /* APOS_DEBUG_H */
