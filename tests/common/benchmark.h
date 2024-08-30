#ifndef KMI_BENCHMARK_H
#define KMI_BENCHMARK_H

#include "sys.h"
#include "start.h"
#include "printf.h"

#define exit() sys_poweroff(SYS_SHUTDOWN)
#define report(start, end, timebase)\
	do {\
		printf("%lld / %lld\n", \
				(long long unsigned)((end) - (start)), \
				(long long unsigned)(timebase)); \
		exit();\
	} while (0)

#endif /* KMI_BENCHMARK_H */
