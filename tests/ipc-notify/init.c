#include <common/test.h>

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(d3);
	check(pid == 0, "illegal pid for init\n");
	check(d0 == SYS_USER_NOTIFY || d0 == SYS_USER_SPAWNED,
			"illegal d0 for init\n");

	if (d0 == SYS_USER_SPAWNED) {
		/* send request to ourselves */
		printf("sending ipc notify to ourselves\n");
		check(tid == 1, "illegal init thread ID\n");
		struct sys_ret r = syscall5(SYS_IPC_NOTIFY, 1, 1, 2, 3, 4);
		printf("returned ipc notify to ourselves\n");

		check(r.s == OK, "illegal notify status\n");
		check(r.id == 0, "illegal notify id\n");
		check(r.a0 != 10, "ipc resp leaked through?\n");
		check(r.a1 != 11, "ipc resp leaked through?\n");
		check(r.a2 != 12, "ipc resp leaked through?\n");
		check(r.a3 != 13, "ipc resp leaked through?\n");

		/* try to notify to non-existing proc */
		enum sys_status s = sys_ipc_notify(200);
		check(s != OK, "got OK return for illegal pid\n");
	}
	else if (d0 == SYS_USER_NOTIFY) {
		printf("caught ipc notify\n");
		check(tid == 1, "illegal tid source\n");
		check(d1 & NOTIFY_SIGNAL, "illegal d1\n");
		check(d2 == 1, "illegal d2\n");

		printf("doing ipc resp in notification\n");
		/* whatever we respond here should not be visible in whoever ran
		 * the notification */
		sys_ipc_resp4(10, 11, 12, 13);
		check(0, "ipc resp failed\n");
	}

	ok();
}
