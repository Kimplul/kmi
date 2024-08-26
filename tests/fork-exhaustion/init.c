#include <common/test.h>

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(pid);
	UNUSED(tid);
	UNUSED(d0);
	UNUSED(d1);
	UNUSED(d2);
	UNUSED(d3);

	check(pid == 0, "illegal pid for init\n");
	id_t our_id = 0;
	while (1) {
		printf("forking\n");
		id_t new_id = sys_fork(&our_id);
		if (new_id < 0)
			break;
	}

	/* if kernel didn't crash, we consider it a success */
	ok();
}
