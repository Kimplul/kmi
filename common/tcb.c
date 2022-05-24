/**
 * @file tcb.c
 * Thread control block handling implementation.
 */

#include <apos/tcb.h>
#include <arch/cpu.h>
#include <apos/mem.h>
#include <apos/conf.h>
#include <apos/pmem.h>
#include <apos/vmem.h>
#include <apos/nodes.h>
#include <apos/types.h>
#include <apos/assert.h>
#include <apos/string.h>
#include <arch/vmem.h>

/* arguably exessively many globals... */
static id_t start_tid;
static size_t num_tids;

static struct tcb **tcbs;

/* if we ever support systems with massive amounts of cpus, this should probably
 * be allocated at runtime */
static struct tcb *cpu_tcb[MAX_CPUS] = { 0 };

void init_tcbs()
{
	/* MM_O1 is 2MiB on riscv64, so 262144 different possible thread ids.
	 * Should be enough, if we're really strapped for memory I might try
	 * something smaller but this is fine for now. */
	tcbs = (struct tcb **)alloc_page(MM_O1, 0);
	num_tids = __o_size(MM_O1) / sizeof(struct tcb *);
	memset(tcbs, 0, __o_size(MM_O1));
}

void destroy_tcbs()
{
	free_page(MM_O1, (pm_t)tcbs);
}

static id_t __alloc_tid(struct tcb *t)
{
	/* TODO: this would need some locking or something... */
	for (size_t i = start_tid; i < num_tids; ++i) {
		if (tcbs[i])
			continue;

		tcbs[i] = t;
		start_tid = i + 1;
		return i;
	}

	return ERR_NF;
}

/* TODO: add error checking */
static vm_t __setup_call_stack(struct tcb *t, size_t bytes)
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

static vm_t __setup_thread_stack(struct tcb *t, size_t bytes)
{
	return alloc_uvmem(t, bytes, VM_V | VM_R | VM_W | VM_U);
}

stat_t alloc_stacks(struct tcb *t)
{
	struct tcb *p = is_proc(t) ? t : t->proc;

	t->thread_stack = __setup_thread_stack(p, __thread_stack_size);
	if (!t->thread_stack)
		return ERR_OOMEM;

	t->call_stack = __setup_call_stack(p, __call_stack_size);
	if (!t->call_stack)
		return ERR_OOMEM;

	/* TODO: this only allows for a global stack size, what if a user wants
	 * per thread stack sizes? */
	t->thread_stack_top = t->thread_stack + __thread_stack_size;
	t->call_stack_top = t->call_stack + __call_stack_size;
	return OK;
}

struct tcb *create_thread(struct tcb *p)
{
	hard_assert(tcbs, 0);

	vm_t bottom = alloc_page(MM_O0, 0);
	/* move tcb to top of kernel stack, keeping alignment in check
	 * (hopefully) */
	/* TODO: check alignment */
	struct tcb *t = (struct tcb *)align_down(
		bottom + __o_size(MM_O0) - sizeof(struct tcb), sizeof(long));
	memset(t, 0, sizeof(struct tcb));

	id_t tid = __alloc_tid(t);
	tcbs[tid] = t;
	t->tid = tid;

	if (likely(p)) {
		t->pid = p->pid;
		t->proc = p;
	} else {
		init_uvmem(t, UVMEM_START, UVMEM_END);
		t->pid = t->tid;
		p = t;
	}

	t->b_r = create_vmem();

	return t;
}

static stat_t __clone_proc(struct tcb *p, struct tcb *n)
{
	/* TODO: clone memory regions, and mark them MR_COW, as well as copy
	 * bm_branch tree but with VM_W off, also at some point write COW
	 * handler */
	return OK;
}

struct tcb *create_proc(struct tcb *p)
{
	hard_assert(tcbs, 0);

	/* create a new thread outside the current process */
	struct tcb *n = create_thread(NULL);
	if (!n)
		return 0;

	if (likely(p))
		__clone_proc(p, n); /* we have a parent thread */

	return n;
}

static stat_t __destroy_thread_data(struct tcb *t)
{
	/* free vmem */
	destroy_vmem(t->b_r);

	/* free associated kernel stack and the structure itself */
	vm_t bottom = align_down((vm_t)t, __o_size(MM_O0));
	free_page(MM_O0, (pm_t)bottom);

	return OK;
}

stat_t destroy_thread(struct tcb *t)
{
	hard_assert(tcbs, ERR_NOINIT);
	hard_assert(!is_proc(t), ERR_INVAL);

	/* remove thread id from list */
	tcbs[t->tid] = 0;

	/* remove thread from process list */
	if (t->next)
		t->next->prev = t->prev;

	if (t->prev)
		t->prev->next = t->next;

	return __destroy_thread_data(t);
}

stat_t destroy_proc(struct tcb *p)
{
	hard_assert(tcbs, ERR_NOINIT);
	hard_assert(is_proc(p), ERR_INVAL);

	for (struct tcb *iter = p; (iter = iter->next);)
		destroy_thread(iter);

	catastrophic_assert(destroy_uvmem(p));
	return __destroy_thread_data(p);
}

struct tcb *cur_tcb()
{
	return cpu_tcb[cpu_id()];
}

struct tcb *cur_proc()
{
	struct tcb *t = cur_tcb();
	if (likely(is_proc(t)))
		return t;
	else
		return t->proc;
}

void use_tcb(struct tcb *t)
{
	cpu_tcb[cpu_id()] = t;
}

struct tcb *get_tcb(id_t tid)
{
	hard_assert(tcbs, 0);

	return tcbs[tid];
}

stat_t clone_tcb_maps(struct tcb *r)
{
	hard_assert(r && is_proc(r), ERR_INVAL);
	struct tcb *t = r;
	while ((t = t->next)) {
		stat_t ret = clone_uvmem(r->b_r, t->b_r);
		if (ret)
			return ret;
	}

	return OK;
}
