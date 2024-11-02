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

	printf("doing spawns...\n");

	int counter = 0;
	while (1) {
		printf("spawning %d\n", counter++);
		id_t new = sys_spawn(spawn, 0);
		if (new < 0)
			break;
	}

	/* could also check for memory leaks but this is good enough for now */
	ok();
}
