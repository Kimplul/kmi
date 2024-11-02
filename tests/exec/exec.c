#include <common/test.h>

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(d2);
	UNUSED(d3);

	printf("pid = %ld, tid = %ld, d0 = %ld, d1 = %ld\n",
			pid, tid, d0, d1);

	check(pid == 0, "unexpected pid for exec\n");
	check(tid == 2, "unexpected tid for exec\n");
	check(d0 == SYS_USER_SPAWNED, "unexpected d0 for exec\n");
	check(d1 == 2, "unexpected d1 for exec\n");
	ok();
}
