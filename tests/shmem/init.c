#include <common/test.h>

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(pid);
	UNUSED(tid);
	UNUSED(d0);
	UNUSED(d1);
	UNUSED(d2);
	UNUSED(d3);

	printf("allocating shared memory\n");
	volatile char *shmem = sys_req_sharedmem(1, VM_R | VM_W);
	check(shmem, "no shared memory?\n");

	*shmem = 'p';

	printf("sharing memory to ourselves\n");
	volatile char *refmem = sys_ref_sharedmem(1, (uintptr_t)shmem, VM_R | VM_W);
	check(refmem[0] == 'p', "wrong mapping?\n");

	printf("freeing shared memory (should fail)\n");
	enum sys_status r = sys_free_mem((uintptr_t)shmem);
	check(r != OK, "illegal freeing succeeded?\n");

	printf("freeing referenced memory\n");
	r = sys_free_mem((uintptr_t)refmem);
	check(r == OK, "legal ref freeing failed?\n");
	check(shmem[0] == 'p', "freeing ref freed owned memory?\n");

	printf("freeing recently nonshared memory\n");
	r = sys_free_mem((uintptr_t)shmem);
	check(r == OK, "legal freeing failed?\n");

	ok();
}
