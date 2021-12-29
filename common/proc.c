#include <apos/proc.h>

/* TODO: add error checking */
vm_t setup_call_stack(struct tcb *t, vm_t start, size_t bytes)
{
	pm_t offset = 0;
	size_t pages = __pages(bytes);
	uint8_t flags = VM_V | VM_R | VM_W | VM_U;
	for(size_t i = 0; i < pages; ++i)
	{
		offset = alloc_page(BASE_PAGE, offset);
		map_vmem(t->b_r, offset, start + BASE_PAGE_SIZE * i, flags, BASE_PAGE);
	}

	return start;
}
