#ifndef KMI_TESTS_H
#define KMI_TESTS_H

#include <kmi/attrs.h>
#include <kmi/types.h>

#include "sys.h"
#include "start.h"
#include "printf.h"

#define error(x, ...)\
	printf("ERROR: " x, ## __VA_ARGS__)

#define check(x, y, ...)\
	if (!(x)) {\
		error(y, ##__VA_ARGS__);\
		sys_poweroff(SYS_SHUTDOWN);\
	}

static inline void ok() {
	printf("OK\n");
	sys_poweroff(SYS_SHUTDOWN);
}

#endif /* KMI_TESTS_H */
