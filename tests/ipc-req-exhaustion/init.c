#include <common/test.h>

long counter = 0;

START(pid, tid, d0, d1, d2, d3)
{
	(void)tid;
	(void)d0;
	(void)d1;
	(void)d2;
	(void)d3;

	check(pid == 0 || pid == 1, "illegal pid for init");
	printf("sending ipc req %zd to ourselves\n", counter++);
	struct sys_ret r = sys_ipc_req4(1, 1, 2, 3, 4);
	if (r.s) {
		printf("ran out of stack space at %ld\n", counter);
		printf("starting to unwind\n");
	}

	printf("unwinding at %ld\n", counter--);

	/* the final resp0 should fail, since we're back in our root process */
	sys_ipc_resp0();

	ok();
}
