#include <common/test.h>

static id_t new_id = 0;
static char *refmem = NULL;

START(pid, tid, d0, d1, d2, d3)
{
	UNUSED(pid);
	UNUSED(tid);
	UNUSED(d0);
	UNUSED(d1);
	UNUSED(d2);
	UNUSED(d3);

	if (pid == 0 && d0 == SYS_USER_SPAWNED) {
		printf("forking\n");
		id_t our_id = 0;
		new_id = sys_fork(&our_id);
		if (new_id == 0) {
			printf("in child, setting up shared memory\n");
			void *p = sys_req_sharedmem(1, VM_R | VM_W);
			check(p, "failed getting shared memory\n");

			printf("referencing shared memory for pid 1\n");
			void *r = sys_ref_sharedmem(1, (uintptr_t)p, VM_R | VM_W);
			check(r, "failed referencing shared memory\n");

			sys_ipc_req1(1, (uintptr_t)r);
			sys_exit(1);
		}

		printf("swapping to child\n");
		enum sys_status r = sys_swap(new_id);
		check(r == OK, "failed swapping to child\n");

		printf("returned from child\n");
		/* child should now be dead */
		check(refmem[0] == 'p', "child dying affected our shared memory?\n");

		printf("freeing shared memory\n");
		r = sys_free_mem((uintptr_t)refmem);
		check(r == OK, "failed freeing referenced memory\n");
	}
	else if (pid == new_id) {
		refmem = (char *)d0;
		refmem[0] = 'p';
		sys_ipc_resp0();
	}
	ok();
}
