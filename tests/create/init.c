#include <common/test.h>

static int data = 0;

void callback(id_t tid, int d0, int d1, int d2, int d3)
{
	printf("hello from new thread\n");
	check(tid == 2, "wrond tid\n");
	check(d0 == 1, "wrong d0\n");
	check(d1 == 2, "wrond d1\n");
	check(d2 == 3, "wrong d2\n");
	check(d3 == 4, "wrong d3\n");
	check(data == 16, "wrong data\n");

	printf("returning to old thread\n");
	sys_swap(1);
	check(0, "swap to old thread failed\n");
}

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(pid);
	UNUSED(tid);
	UNUSED(d0);
	UNUSED(d1);
	UNUSED(d2);
	UNUSED(d3);

	printf("setting data to non-zero value\n");
	data = 16;

	printf("creating new thread\n");
	id_t new_thread = sys_create((uintptr_t)callback, 1, 2, 3, 4);
	check(new_thread > 0, "create failed\n");

	printf("swapping to new thread\n");
	enum sys_status r = sys_swap(new_thread);
	check(r == OK, "swap to new thread failed\n");

	ok();
}
