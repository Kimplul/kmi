#include <common/benchmark.h>

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(tid);
	UNUSED(d0);
	UNUSED(d1);
	UNUSED(d2);
	UNUSED(d3);

	if (pid == 0) {
		uint64_t timebase = sys_timebase();
		uint64_t start = sys_ticks();

		for (size_t i = 0; i < 1000; ++i)
			sys_ipc_req0(1);

		uint64_t end = sys_ticks();
		report(start, end, timebase);
	}
	else if (pid == 1) {
		sys_ipc_resp0();
	}
}
