#include <common/test.h>

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(pid);
	UNUSED(tid);
	UNUSED(d1);
	UNUSED(d2);
	UNUSED(d3);

	if (d0 == SYS_USER_SPAWNED) {
		size_t prev_ram = sys_conf_get(CONF_RAM_USAGE, 0);
		printf("ram usage before new process: %zx\n", prev_ram);

		printf("forking\n");
		id_t our_tid = 0;
		id_t new_id = sys_fork(&our_tid);
		check(new_id > 0, "error from fork\n");

		/* not in general but in this case */
		check(new_id != 0, "in child, should never be in child\n");

		printf("checking detach of nonsense id\n");
		enum sys_status r = sys_detach(200);
		check(r != OK, "nonsense id succeeded?\n");

		printf("detaching child\n");
		r = sys_detach(new_id);
		check(r == OK, "detaching child failed\n");

		printf("swapping to detached/orphaned thread\n");
		r = sys_swap(new_id);
		check(r == OK, "swapping to detached thread failed\n");

		size_t new_ram = sys_conf_get(CONF_RAM_USAGE, 0);
		printf("ram usage after new process: %zx\n", new_ram);

		check(new_ram == prev_ram, "fork/detach leaked memory\n");
		ok();
	}
	else if (d0 == SYS_USER_ORPHANED) {
		printf("exiting with nonsense swap id\n");
		enum sys_status r = sys_exit(200);
		check(r != OK, "nonsense exit succeeded (but returned...?)\n");

		printf("exiting with sensible swap id\n");
		r = sys_exit(1);
		check(0, "exit with sensible id returned\n");
	}

	check(0, "missed SYS_USER_ORPHANED\n");
}
