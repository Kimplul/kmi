/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file tcb.c
 * Thread control block handling implementation.
 */

#include <apos/tcb.h>
#include <apos/mem.h>
#include <apos/conf.h>
#include <apos/pmem.h>
#include <apos/vmem.h>
#include <apos/nodes.h>
#include <apos/types.h>
#include <apos/assert.h>
#include <apos/string.h>
#include <apos/canary.h>

#include <arch/cpu.h>
#include <arch/vmem.h>
#include <arch/proc.h>

/* arguably exessively many globals... */
/** Thread ID to start looking from when allocating new ID. */
static id_t start_tid;

/** Total number of possible thread IDs. */
static id_t num_tids;

/** Pointer to array of \ref tcb structures. Length of the array is \c num_tids.*/
static struct tcb **tcbs;

/**
 * Array of thread control block associated with each cpu.
 *
 * \todo If we ever support systems with massive amounts of cpus, this should probably
 * be allocated at runtime.
 */
static struct tcb *__cpu_tcb[MAX_CPUS] = { 0 };

void init_tcbs()
{
	/* MM_O1 is 2MiB on riscv64, so 262144 different possible thread ids.
	 * Should be enough, if we're really strapped for memory I might try
	 * something smaller but this is fine for now. */
	tcbs = (struct tcb **)alloc_page(MM_O1);
	num_tids = order_size(MM_O1) / sizeof(struct tcb *);
	memset(tcbs, 0, order_size(MM_O1));
}

void destroy_tcbs()
{
	free_page(MM_O1, (pm_t)tcbs);
}

/**
 * Allocate a new thread ID.
 *
 * @param t Thread to allocate new ID to.
 * @return Allocated ID.
 */
static id_t __alloc_tid(struct tcb *t)
{
	id_t stop_tid = start_tid - 1;
	/** \todo this would need some locking or something... */
	for (id_t i = start_tid; 1; ++i) {
		if (i == ID_MAX)
			i = 0;

		/* we're completely full */
		if (i == stop_tid)
			return ERR_NF;

		if (tcbs[i] || i == 0)
			continue;

		tcbs[i] = t;
		start_tid = i + 1;
		return i;
	}

	return ERR_NF;
}

/**
 * Setup RPC stack.
 *
 * RPC stack is local to each thread, and should not be visible to other threads
 * in the same process. Currently maps the RPC stack in BASE_PAGE increments, to
 * hopefully allow us to later quickly disallow access to programs lower down in
 * the RPC call chain by turning off all stack pages lower than the current
 * stack pointer. We shall see if this actually works or not.
 *
 * @param t Thread to setup RPC stack for.
 * @param bytes Minimum size of RPC stack.
 * @return Base of allocated RPC stack.
 *
 * \todo add error checking */
static vm_t __setup_rpc_stack(struct tcb *t, size_t bytes)
{
	pm_t offset = 0;
	size_t pages = __pages(bytes);
	vmflags_t flags = VM_V | VM_R | VM_W | VM_U;
	for (size_t i = 1; i <= pages; ++i) {
		offset = alloc_page(BASE_PAGE);
		map_vpage(t->rpc.vmem, offset,
		          RPC_STACK_TOP - BASE_PAGE_SIZE * i,
		          flags, BASE_PAGE);
	}
	t->rpc_stack = RPC_STACK_TOP;
	return RPC_STACK_TOP - BASE_PAGE_SIZE * pages;
}

/**
 * Setup thread stack.
 *
 * @param t Thread to setup stack for.
 * @param bytes Minimum size of stack.
 * @return Base of allocated stack.
 */
static vm_t __setup_thread_stack(struct tcb *t, size_t bytes)
{
	return alloc_uvmem(t, bytes, VM_V | VM_R | VM_W | VM_U);
}

stat_t alloc_stack(struct tcb *t)
{
	/* get parent process */
	struct tcb *p = get_tcb(t->eid);

	t->thread_stack = __setup_thread_stack(p, __thread_stack_size);
	if (!t->thread_stack)
		return ERR_OOMEM;

	/** \todo this only allows for a global stack size, what if a user wants
	 * per thread stack sizes? */
	t->thread_stack_top = t->thread_stack + __thread_stack_size;
	return OK;
}

struct tcb *create_thread(struct tcb *p)
{
	hard_assert(tcbs, 0);

	vm_t bottom = alloc_page(KERNEL_STACK_PAGE_ORDER);
	/* move tcb to top of kernel stack, keeping alignment in check
	 * (hopefully) */
	/** \todo check alignment */
	struct tcb *t = (struct tcb *)align_down(
		bottom + order_size(MM_O0) - sizeof(struct tcb), sizeof(long));
	memset(t, 0, sizeof(struct tcb));

	id_t tid = __alloc_tid(t);
	tcbs[tid] = t;
	t->tid = tid;

	if (likely(p)) {
		t->pid = p->pid;
		/** @todo I'm assuming two threads can share the same vmem
		 * structure, this works on riscv but in the event that other
		 * systems don't we can easily turn this into a clone_uvmem. */
		t->proc.vmem = p->proc.vmem;
	} else {
		init_uvmem(t, UVMEM_START, UVMEM_END);
		t->proc.vmem = create_vmem();
		t->pid = t->tid;
		t->rid = t->tid;
		p = t;
	}

	t->eid = t->pid;
	t->rid = p->rid;
	t->rpc.vmem = create_vmem();
	__setup_rpc_stack(t, __call_stack_size);

	t->regs = (vm_t)t;

	set_canary(t);
	return t;
}

/**
 * Copy process.
 *
 * @param p Parent process.
 * @param n New process.
 * @return \ref OK.
 */
static stat_t __copy_proc(struct tcb *p, struct tcb *n)
{
	/** @todo setup rpc stack stuff */
	/** @todo I think keeping track of userspace stack stuff is unnecessary,
	 * unless we want unlimited stack size but that sounds dumb. Anycase, we
	 * need to duplicate stack info, whatever we do. */
	n->exec = p->exec;
	n->callback = p->callback;
	n->thread_stack = p->thread_stack;
	n->thread_stack_top = p->thread_stack_top;

	clone_regs(n, p);
	copy_caps(n->caps, p->caps);
	return clone_mem_regions(n, p);
}

struct tcb *create_proc(struct tcb *p)
{
	hard_assert(tcbs, 0);

	/* create a new thread outside the current process */
	struct tcb *n = create_thread(NULL);
	if (!n)
		return 0;

	if (p)
		__copy_proc(p, n); /* we have a parent thread */

	return n;
}

/**
 * Destroy data associated with thread.
 *
 * @param t Thread whose data to destroy.
 * @return \ref OK.
 */
static stat_t __destroy_thread_data(struct tcb *t)
{
	/* free rpc vmem */
	destroy_vmem(t->rpc.vmem);

	/* free associated kernel stack and the structure itself */
	vm_t bottom = align_down((vm_t)t, order_size(MM_O0));
	free_page(MM_O0, (pm_t)bottom);

	/** \todo free stacks */

	return OK;
}

stat_t destroy_thread(struct tcb *t)
{
	hard_assert(tcbs, ERR_NOINIT);
	hard_assert(!is_proc(t), ERR_INVAL);

	/* remove thread id from list */
	tcbs[t->tid] = 0;

	/* remove thread from process list */
	detach_proc(get_rproc(t), t);

	return __destroy_thread_data(t);
}

stat_t destroy_proc(struct tcb *p)
{
	hard_assert(tcbs, ERR_NOINIT);
	hard_assert(is_proc(p), ERR_INVAL);

	for (struct tcb *iter = p; (iter = iter->proc.next);)
		destroy_thread(iter);

	catastrophic_assert(destroy_uvmem(p));
	return __destroy_thread_data(p);
}

stat_t attach_rpc(struct tcb *r, struct tcb *t)
{
	hard_assert(r != t, ERR_INVAL);
	struct tcb *next = r->server.next;
	t->rpc.next = next;

	if (next) { next->rpc.prev = t; }

	t->rpc.prev = r;
	r->server.next = t;
	return OK;
}

stat_t attach_proc(struct tcb *r, struct tcb *t)
{
	hard_assert(r != t, ERR_INVAL);
	struct tcb *next = r->proc.next;
	t->proc.next = next;

	if (next) { next->proc.prev = t; }

	t->proc.prev = r;
	r->proc.next = t;
	return OK;
}

stat_t detach_rpc(struct tcb *r, struct tcb *t)
{
	/* rpc handling is slightly more complex since we have separate members
	 * for server and rpc contexts, where server is the server that
	 * currently hosts some number of rpc guests. */
	hard_assert(r != t, ERR_INVAL);
	struct tcb *prev = t->rpc.prev;
	struct tcb *next = t->rpc.next;

	if (prev == r) { r->server.next = next; }
	else if (prev) { prev->rpc.next = next; }

	if (next) { next->rpc.prev = prev; }

	return OK;
}

stat_t detach_proc(struct tcb *r, struct tcb *t)
{
	MAYBE_UNUSED(r);
	hard_assert(r != t, ERR_INVAL);
	struct tcb *prev = t->proc.prev;
	struct tcb *next = t->proc.next;

	if (prev) { prev->proc.next = next; }
	if (next) { next->proc.prev = prev; }

	return OK;
}

/* weak to allow optimisation on risc-v, but provide fallback for future */
__weak struct tcb *cur_tcb()
{
	return cpu_tcb(cpu_id());
}

struct tcb *cpu_tcb(id_t cpu_id)
{
	return __cpu_tcb[cpu_id];
}

struct tcb *cur_proc()
{
	struct tcb *t = cur_tcb();
	return get_tcb(t->pid);
}

struct tcb *eff_proc()
{
	struct tcb *t = cur_tcb();
	return get_tcb(t->eid);
}

void use_tcb(struct tcb *t)
{
	cpu_assign(t);

	__cpu_tcb[t->cpu_id] = t;

	use_vmem(t->proc.vmem);
}

struct tcb *get_tcb(id_t tid)
{
	hard_assert(tcbs, 0);

	if (tid <= 0 || tid >= num_tids)
		return NULL;

	return tcbs[tid];
}

stat_t clone_rpc_maps(struct tcb *r)
{
	hard_assert(r && is_proc(r), ERR_INVAL);
	struct tcb *t = r->server.next;
	if (!t)
		return OK;

	do {
		clone_uvmem(r->proc.vmem, t->rpc.vmem);
	} while ((t = t->rpc.next));

	return OK;
}

stat_t clone_proc_maps(struct tcb *r)
{
	hard_assert(r && is_proc(r), ERR_INVAL);
	struct tcb *t = r;
	while ((t = t->proc.next))
		clone_uvmem(r->proc.vmem, t->proc.vmem);

	return OK;
}

void set_return(struct tcb *t, vm_t v)
{
	t->exec = v;
}

bool running(struct tcb *t)
{
	return cpu_tcb(t->cpu_id) == t;
}

/**
 * Mark rpc stack between \p start and \p end inaccessible.
 *
 * @param t Thread whose rpc stack to modify.
 * @param start Start address of rpc stack to mark inaccessible.
 * @param end End address of rpc stack to mark inaccessible.
 */
static void mark_rpc_inaccessible(struct tcb *t, vm_t start, vm_t end)
{
	size_t page_size = BASE_PAGE_SIZE;
	size_t size = end - start;
	size_t pages = size / page_size;
	while (pages--)
		clear_vpage_flags(t->rpc.vmem, start + pages * page_size, VM_U);
}

/**
 * Mark rpc stack between \p start and \p end accessible.
 *
 * @param t Thread whose rpc stack to modify.
 * @param start Start address of rpc stack to mark accessible.
 * @param end End address of rpc stack to mark accessible.
 */
static void mark_rpc_accessible(struct tcb *t, vm_t start, vm_t end)
{
	size_t page_size = BASE_PAGE_SIZE;
	size_t size = end - start;
	size_t pages = size / page_size;
	while (pages--)
		set_vpage_flags(t->rpc.vmem, start + pages * page_size, VM_U);
}

/** Structure for maintaingin the required context data for an rpc call. */
struct call_ctx {
	/** Execution continuation point. */
	vm_t exec;

	/** Register save area. */
	vm_t regs;

	/** Position in rpc stack. */
	vm_t rpc_stack;

	/** Effective process ID. */
	id_t eid;

	/** Current process ID. */
	id_t pid;
};

void enter_rpc(struct tcb *t)
{
	vm_t rpc_stack = t->rpc_stack;
	if (is_rpc(t))
		/** @todo what if user uses their own stack? Or is a dick and
		 * sets the stack pointer to RPC_STACK_TOP or something? It'll
		 * likely only cause a fuckup in the process who did the dumb
		 * thing, so maybe just consider it user error? Except by
		 * causing the stack of the next rpc to run out of memory... */
		rpc_stack = align_down(get_stack(t), BASE_PAGE_SIZE);


	/* make sure updates are visible when swapping to the new virtual memory */
	mark_rpc_inaccessible(t, rpc_stack, t->rpc_stack);
	use_vmem(t->rpc.vmem);

	struct call_ctx *ctx = (struct call_ctx *)(rpc_stack) - 1;
	ctx->exec = t->exec;
	ctx->pid = t->pid;
	ctx->eid = t->eid;
	ctx->regs = t->regs;
	ctx->rpc_stack = rpc_stack;

	/** @todo if we run out of rpc_stack space we should just stop, likely
	 * return a status? */
	rpc_stack -= BASE_PAGE_SIZE;

	/** @todo what if each stack is only some number of pages, and if a proc
	 * goes over the limit is is seen as programming error? Possibly user
	 * configurable number as well, might actually use the config subsystem
	 * :D
	 * In such a case it would probably be smarter to mark all pages
	 * inaccessible at first, and then mark the first page accessible. If
	 * the process needs more stack space it'll cause a paging exception,
	 * we'll handle it separately and if the process isn't going over the
	 * limit just give it more.
	 * */
	t->rpc_stack = rpc_stack;
	t->regs = (vm_t)ctx;
	set_stack(t, rpc_stack);
}

void leave_rpc(struct tcb *t)
{
	vm_t rpc_stack = t->rpc_stack + BASE_PAGE_SIZE;
	struct call_ctx *ctx = (struct call_ctx *)(rpc_stack) - 1;
	set_return(t, ctx->exec);
	mark_rpc_accessible(t, t->rpc_stack, ctx->rpc_stack);
	t->rpc_stack = ctx->rpc_stack;
	t->pid = ctx->pid;
	t->eid = ctx->eid;
	t->regs = ctx->regs;
}

bool enough_rpc_stack(struct tcb *t)
{
	vm_t top = RPC_STACK_BASE + __call_stack_size;
	vm_t rpc_stack = t->rpc_stack + BASE_PAGE_SIZE;

	return top - rpc_stack >= __call_stack_size / RPC_STACK_RATIO;
}
