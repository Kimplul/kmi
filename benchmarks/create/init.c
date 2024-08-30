#include <common/benchmark.h>

void callback()
{
	sys_exit(1);
}

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
		id_t new_id = sys_create((uintptr_t)callback, 0, 0, 0, 0);
		sys_swap(new_id);
	}

	uint64_t end = sys_ticks();
	report(start, end, timebase);
}
