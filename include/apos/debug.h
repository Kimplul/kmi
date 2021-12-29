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

#else

#define dbg(...)
#define dbg_init(...)
#define dbg_from_fdt(...)

#endif /* DEBUG */

#endif /* APOS_DEBUG_H */
