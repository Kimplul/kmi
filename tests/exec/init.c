#include <common/test.h>
#include <common/cpio.h>

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
	printf("forking\n");
	id_t new_id = sys_fork(&our_id);
	check(new_id >= 0, "error from fork\n");

	if (new_id == 0) {
		printf("hello from child with pid %ld\n", (long int)our_id);
		printf("finding exec in cpio archive %lx...\n", d2);
		struct cpio_header *cp = cpio_find_file((const void *)d2,
				"exec", sizeof("exec") - 1);

		check(cp, "couldn't find exec?\n");
		long name_len = convnum(cp->c_namesize, 8, 16);
		uintptr_t exec = (uintptr_t)(cp) + align_up(sizeof(struct cpio_header) + name_len, 4);
		printf("found exec at %lx\n", exec);

		printf("doing exec...\n");
		sys_exec(exec, 0);
		error("exec failed\n");
	}

	printf("hello from parent\n");
	sys_swap(new_id);
	check(0, "failed swapping to child\n");
}
