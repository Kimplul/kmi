#ifndef APOS_DEBUG_H
#define APOS_DEBUG_H

#include <apos/attrs.h>

#ifdef DEBUG
enum serial_dev_t {
	/* only the NS16550A and compatible at the moment */
	NS16550A,
};

struct dbg_info_t {
	void *dbg_ptr;
	enum serial_dev_t dev;
};

void __fmt(1, 2) dbg(const char *fmt, ...);
void dbg_init(void *pt, enum serial_dev_t dev);
struct dbg_info_t dbg_from_fdt(void *fdt);

#else

#define dbg(...)
#define dbg_init(...)

#endif /* DEBUG */

#endif /* APOS_DEBUG_H */
