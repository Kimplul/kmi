#include <common/benchmark.h>

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(pid);
	UNUSED(tid);
	UNUSED(d0);
	UNUSED(d1);
	UNUSED(d2);
	UNUSED(d3);

	uint64_t timebase = sys_timebase();
	uint64_t start = sys_ticks();

	for (size_t i = 0; i < 1000; ++i) {
		id_t our_tid = 0;
		id_t new_id = sys_fork(&our_tid);
		if (new_id == 0) {
			/* child instantly dies */
			sys_exit(1);
		}
		sys_swap(new_id);
	}

	uint64_t end = sys_ticks();
	report(start, end, timebase);
}
