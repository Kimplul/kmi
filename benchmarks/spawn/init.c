#include <common/benchmark.h>
#include <common/cpio.h>

void callback()
{
	sys_exit(1);
}

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(pid);
	UNUSED(tid);
	UNUSED(d0);
	UNUSED(d1);
	UNUSED(d2);
	UNUSED(d3);

	if (d0 == SYS_USER_ORPHANED) {
		sys_exit(1);
	}

	printf("finding spawn in cpio archive %lx...\n", d2);
	struct cpio_header *cp = cpio_find_file((const void *)d2,
			"spawn", sizeof("spawn") - 1);

	if (!cp) {
		printf("couldn't find spawn?\n");
		report(0, 0, 0);
	}

	long name_len = convnum(cp->c_namesize, 8, 16);
	uintptr_t spawn = (uintptr_t)(cp) + align_up(sizeof(struct cpio_header) + name_len, 4);
	printf("found spawn at %lx\n", spawn);

	uint64_t timebase = sys_timebase();
	uint64_t start = sys_ticks();

	for (size_t i = 0; i < 1000; ++i) {
		id_t new_id = sys_spawn(spawn, 0);
		sys_detach(new_id);
		sys_swap(new_id);
	}

	uint64_t end = sys_ticks();
	report(start, end, timebase);
}
