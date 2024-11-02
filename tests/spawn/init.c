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
	printf("finding spawn in cpio archive %lx...\n", d2);
	struct cpio_header *cp = cpio_find_file((const void *)d2,
			"spawn", sizeof("spawn") - 1);

	check(cp, "couldn't find spawn?\n");
	long name_len = convnum(cp->c_namesize, 8, 16);
	uintptr_t spawn = (uintptr_t)(cp) + align_up(sizeof(struct cpio_header) + name_len, 4);
	printf("found spawn at %lx\n", spawn);

	printf("doing spawn...\n");
	id_t new = sys_spawn(spawn, 0);
	check(new > 1, "spawn failed?\n");

	printf("giving spawn CAP_POWER\n");
	enum sys_status s = sys_set_cap(new, CAP_POWER);
	check(s == OK, "CAP_POWER failed?\n");

	printf("swapping to spawn\n");
	s = sys_swap(new);
	check(s == OK, "swap to spawn failed?\n");
}
