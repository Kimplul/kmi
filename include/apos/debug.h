#ifndef APOS_DEBUG_H
#define APOS_DEBUG_H

#include <apos/attrs.h>

#ifdef DEBUG
enum serial_dev_t {
	/* only the NS16550A and compatible at the moment */
	NS16550A,
};

void __fmt(1, 2) dbg(const char *fmt, ...);
void dbg_init(void *pt, enum serial_dev_t dev);

#else

#define dbg(...)
#define dbg_init(...)

#endif /* DEBUG */

#endif /* APOS_DEBUG_H */
