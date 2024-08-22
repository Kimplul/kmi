#include <common/test.h>

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(pid);
	UNUSED(tid);
	UNUSED(d0);
	UNUSED(d1);
	UNUSED(d2);
	UNUSED(d3);

	/** @todo fork until we run out of memory? */
	check(pid == 0, "illegal pid for init\n");
	id_t our_id = 0;
	printf("forking\n");
	id_t new_id = sys_fork(&our_id);
	check(new_id >= 0, "error from fork\n");

	if (new_id == 0) {
		printf("hello from child with pid %ld\n", (long int)our_id);
		ok();
	}

	printf("hello from parent\n");
	sys_swap(new_id);
	check(0, "failed swapping to child\n");
}
