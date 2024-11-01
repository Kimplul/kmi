#include <common/test.h>

START(pid, tid, d0, d1, d2, d3)
{
	check(pid == 0 || pid == 1, "illegal pid for init\n");
	if (pid == 0) {
		printf("creating pid 2\n");
		id_t id1 = sys_fork(NULL);
		check(id1 == 2, "unexpected pid?\n");

		printf("creating pid 3\n");
		id_t id2 = sys_fork(NULL);
		check(id2 == 3, "unexpected pid?\n");

		struct sys_ret r = sys_ipc_req4(2, 1, 2, 3, 4);
		check(r.s == 0, "failed request?\n");
		check(r.id == 3, "wrong responder?\n");
		check(r.a0 == 5, "wrong a0?\n");
		check(r.a1 == 6, "wrong a1?\n");
		check(r.a2 == 7, "wrong a2?\n");
		check(r.a3 == 8, "wrong a3?\n");
	}
	else if (pid == 1 && d0 == 1) {
		printf("pid 2 caught ipc req\n");
		check(pid == 1, "illegal pid source\n");
		check(tid == 1, "illegal tid source\n");
		check(d0 == 1, "illegal d0\n");
		check(d1 == 2, "illegal d1\n");
		check(d2 == 3, "illegal d2\n");
		check(d3 == 4, "illegal d3\n");

		printf("pid 2 kicking\n");
		sys_ipc_kick4(3, 5, 6, 7, 8);
		error("pid 2 ipc_tail failed\n");
	}
	else if (pid == 1 && d0 == 5) {
		printf("pid 3 caught ipc kick\n");
		check(pid == 1, "illegal pid source\n");
		check(tid == 1, "illegal tid source\n");
		check(d0 == 5, "illegal d0\n");
		check(d1 == 6, "illegal d1\n");
		check(d2 == 7, "illegal d2\n");
		check(d3 == 8, "illegal d3\n");

		printf("pid 3 responding\n");
		sys_ipc_resp4(5, 6, 7, 8);
		error("pid 3 ipc_resp failed\n");
	}
	else {
		error("weird pid: %ld\n", pid);
	}

	ok();
}
