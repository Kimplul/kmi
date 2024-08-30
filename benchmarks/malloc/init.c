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
		void *p = sys_req_mem(1, VM_R | VM_W);
		sys_free_mem((uintptr_t)p);
	}

	uint64_t end = sys_ticks();
	report(start, end, timebase);
}
