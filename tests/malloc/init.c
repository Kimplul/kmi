#include <common/test.h>

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(pid);
	UNUSED(tid);
	UNUSED(d0);
	UNUSED(d1);
	UNUSED(d2);
	UNUSED(d3);

	const long nallocs = 200000;
	printf("allocating space for pointers...\n");
	/* 128 MiB / 4K */
	void **allocs = sys_req_mem(nallocs * sizeof(void *), VM_W | VM_R);
	check(allocs, "initial allocation failed\n");

	printf("allocating memory until we run out...\n");

	long i = 0;
	while (1) {
		check(i < nallocs, "not enough pointers for malloc test\n");
		char *p = sys_req_mem(1, VM_W | VM_R);
		if (!p)
			break;

		/* check that we actually got a page */
		*p = 'a';
		allocs[i++] = p;
	}

	printf("ran out of memory, freeing...\n");
	for (; i >= 0; --i)
		sys_free_mem((uintptr_t)allocs[i]);

	printf("freed all memory, doing one last allocation to check we're good...\n");
	/* not super exhaustive but would hopefully go haywire if the page
	 * allocator got corrupted or something */
	char *p = sys_req_mem(1, VM_W | VM_R);
	*p = 'b';
	sys_free_mem((uintptr_t)p);
	sys_free_mem((uintptr_t)allocs);
	ok();
}
