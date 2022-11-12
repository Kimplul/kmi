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
static size_t num_tids;

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
	/** \todo this would need some locking or something... */
	for (size_t i = start_tid; i < num_tids; ++i) {
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
	/** @todo should there be in-kernel child tracking? */
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

/**
 * Convenience marco for defining function to attach a thread to either process
 * or RPC context.
 *
 * @param name name of function to define.
 * @param type Field name of type \c tcb_ctx.
 */
#define DEFINE_ATTACH(name, type)                  \
	stat_t name(struct tcb *r, struct tcb *t)  \
	{                                          \
		hard_assert(r != t, ERR_INVAL);    \
		struct tcb *next = r->type.next;   \
		t->type.next = next;               \
                                                   \
		if (next) { next->type.prev = t; } \
                                                   \
		t->type.prev = r;                  \
		r->type.next = t;                  \
		return OK;                         \
	}

DEFINE_ATTACH(attach_rpc, rpc);
DEFINE_ATTACH(attach_proc, proc);

/**
 * Convenience marco for defining function to detach a thread from either process
 * or RPC context.
 *
 * @param name name of function to define.
 * @param type Field name of type \c tcb_ctx.
 */
#define DEFINE_DETACH(name, type)                     \
	stat_t name(struct tcb *r, struct tcb *t)     \
	{                                             \
		MAYBE_UNUSED(r);                      \
		hard_assert(r != t, ERR_INVAL);       \
		struct tcb *prev = t->type.prev;      \
		struct tcb *next = t->type.next;      \
                                                      \
		if (prev) { prev->type.next = next; } \
		if (next) { next->type.prev = prev; } \
                                                      \
		return OK;                            \
	}

DEFINE_DETACH(detach_rpc, rpc);
DEFINE_DETACH(detach_proc, proc);

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

	return tcbs[tid];
}

stat_t clone_rpc_maps(struct tcb *r)
{
	hard_assert(r && is_proc(r), ERR_INVAL);
	struct tcb *t = r;
	while ((t = t->rpc.next)) {
		stat_t ret = clone_uvmem(r->proc.vmem, t->rpc.vmem);
		if (ret)
			return ret;
	}

	return OK;
}

stat_t clone_proc_maps(struct tcb *r)
{
	hard_assert(r && is_proc(r), ERR_INVAL);
	struct tcb *t = r;
	while ((t = t->proc.next)) {
		stat_t ret = clone_uvmem(r->proc.vmem, t->proc.vmem);
		if (ret)
			return ret;
	}

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

struct call_ctx {
	vm_t exec;
	vm_t rpc_stack;
	id_t eid, pid;
};

void save_context(struct tcb *t)
{
	vm_t rpc_stack = t->rpc_stack;
	if (is_rpc(t))
		/** @todo what if user uses their own stack? */
		rpc_stack = align_down(get_stack(t), BASE_PAGE_SIZE);

	rpc_stack -= BASE_PAGE_SIZE;

	struct call_ctx *ctx = (struct call_ctx *)rpc_stack;
	ctx->exec = t->exec;
	ctx->pid = t->pid;
	ctx->eid = t->eid;
	ctx->rpc_stack = rpc_stack + BASE_PAGE_SIZE;
	save_regs(t, ctx + 1);

	mark_rpc_inaccessible(t, rpc_stack, t->rpc_stack);
	t->rpc_stack = rpc_stack;
}

void load_context(struct tcb *t)
{
	vm_t rpc_stack = t->rpc_stack;
	struct call_ctx *ctx = (struct call_ctx *)rpc_stack;
	set_return(t, ctx->exec);
	mark_rpc_accessible(t, t->rpc_stack, ctx->rpc_stack);
	load_regs(ctx + 1, t);
	t->rpc_stack = ctx->rpc_stack;
	t->pid = ctx->pid;
	t->eid = ctx->eid;
}
