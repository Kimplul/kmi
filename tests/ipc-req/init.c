#include <common/test.h>

START(pid, tid, d0, d1, d2, d3)
{
	check(pid == 0 || pid == 1, "illegal pid for init");
	if (pid == 0) {
		/* send request to ourselves */
		printf("sending ipc req to ourselves\n");
		struct sys_ret r = sys_ipc_req4(1, 1, 2, 3, 4);
		printf("returned ipc req to ourselves\n");
		check(r.s == OK, "not OK return\n");
		check(r.id == 1, "not OK response ID\n");
		check(r.a0 == 1, "not OK d0 response\n");
		check(r.a1 == 2, "not OK d1 response\n");
		check(r.a2 == 3, "not OK d2 response\n");
		check(r.a3 == 4, "not OK d3 response\n");

		/* try to request to non-existing proc */
		r = sys_ipc_req0(200);
		check(r.s != OK, "got OK return for illegal pid\n");
	}
	else if (pid == 1) {
		printf("caught ipc req\n");
		check(pid == 1, "illegal pid source\n");
		check(tid == 1, "illegal tid source\n");
		check(d0 == 1, "illegal d0\n");
		check(d1 == 2, "illegal d1\n");
		check(d2 == 3, "illegal d2\n");
		check(d3 == 4, "illegal d3\n");

		printf("sending response\n");
		sys_ipc_resp4(1, 2, 3, 4);
	}

	ok();
}
