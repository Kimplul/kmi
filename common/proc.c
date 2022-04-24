#include <apos/elf.h>
#include <apos/proc.h>
#include <apos/conf.h>
#include <apos/string.h>
#include <apos/initrd.h>
#include <arch/arch.h>

/* TODO: add error checking */
static vm_t setup_call_stack(struct tcb *t, size_t bytes)
{
	pm_t offset = 0;
	size_t pages = __pages(bytes);
	vmflags_t flags = VM_V | VM_R | VM_W | VM_U;
	for (size_t i = 1; i <= pages; ++i) {
		offset = alloc_page(BASE_PAGE, offset);
		map_vpage(t->b_r, offset, PROC_STACK_TOP - BASE_PAGE_SIZE * i,
		          flags, BASE_PAGE);
	}

	return PROC_STACK_TOP - BASE_PAGE_SIZE * pages;
}

static vm_t setup_proc_stack(struct tcb *t, size_t bytes)
{
	return alloc_uvmem(t, bytes, VM_V | VM_R | VM_W | VM_U);
}

stat_t init_proc(void *fdt, struct vm_branch *b)
{
	init_tcbs();

	/* todo: cleanup or something */
	struct tcb *t = new_thread();
	if (!t)
		return ERR_OOMEM;
	t->b_r = b;

	init_uvmem(t, UVMEM_START, UVMEM_END);

	/* the binary gets to choose first what memory regions it requires */
	t->entry = load_elf(t, get_init_base(fdt));
	if (!t->entry)
		return ERR_ADDR;

	t->proc_stack = setup_proc_stack(t, __proc_stack_size);
	if (!t->proc_stack)
		return ERR_ADDR;

	t->proc_stack_top = t->proc_stack + __proc_stack_size;

	t->call_stack = setup_call_stack(t, __call_stack_size);
	if (!t->call_stack)
		return ERR_ADDR;
	t->call_stack_top = t->call_stack + __call_stack_size;

	flush_tlb();

	/* set current tcb */
	use_tcb(t);

	/* TODO: move fdt into process space */
	return jump_to_userspace(t, 1, 0);
}
