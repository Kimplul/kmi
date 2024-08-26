#include <common/test.h>

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(pid);
	UNUSED(tid);
	UNUSED(d0);
	UNUSED(d1);
	UNUSED(d2);
	UNUSED(d3);

	size_t old_ram = 0;
	while (1) {
		printf("creating new thread\n");
		id_t new_thread = sys_create((uintptr_t)_start, 1, 2, 3, 4);
		if (new_thread < 0)
			break;

		old_ram = sys_conf_get(CONF_RAM_USAGE, 0);
	}

	/* this might not be guaranteed due to how we're currently mapping the
	 * rpc stack, but good enough for now */
	size_t ram = sys_conf_get(CONF_RAM_USAGE, 0);
	check(ram == old_ram, "'leaked' memory building thread without enough memory\n");

	/* no crash is good enough */
	ok();
}
