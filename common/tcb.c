/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

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
	num_tids = order_size(MM_O1) / sizeof(struct tcb *);
	memset(tcbs, 0, order_size(MM_O1));
}

void destroy_tcbs()
{
	free_page(MM_O1, (pm_t)tcbs);
}

static id_t __alloc_tid(struct tcb *t)
{
	/** \todo this would need some locking or something... */
	for (size_t i = start_tid; i < num_tids; ++i) {
		if (tcbs[i])
			continue;

		tcbs[i] = t;
		start_tid = i + 1;
		return i;
	}

	return ERR_NF;
}

/** \todo add error checking */
static vm_t __setup_rpc_stack(struct tcb *t, size_t bytes)
{
	pm_t offset = 0;
	size_t pages = __pages(bytes);
	vmflags_t flags = VM_V | VM_R | VM_W | VM_U;
	for (size_t i = 1; i <= pages; ++i) {
		offset = alloc_page(BASE_PAGE, offset);
		map_vpage(t->proc.vmem, offset,
		          RPC_STACK_TOP - BASE_PAGE_SIZE * i,
		          flags, BASE_PAGE);
	}

	return RPC_STACK_TOP - BASE_PAGE_SIZE * pages;
}

static vm_t __setup_thread_stack(struct tcb *t, size_t bytes)
{
	return alloc_uvmem(t, bytes, VM_V | VM_R | VM_W | VM_U);
}

stat_t alloc_stacks(struct tcb *t)
{
	/* get parent process */
	struct tcb *p = get_tcb(t->eid);

	t->thread_stack = __setup_thread_stack(p, __thread_stack_size);
	if (!t->thread_stack)
		return ERR_OOMEM;

	/* rpc stack always starts at the same place in vmem.
	 * \todo: is this a security issue? */
	if (!__setup_rpc_stack(p, __call_stack_size))
		return ERR_OOMEM;

	/** \todo this only allows for a global stack size, what if a user wants
	 * per thread stack sizes? */
	t->thread_stack_top = t->thread_stack + __thread_stack_size;
	return OK;
}

struct tcb *create_thread(struct tcb *p)
{
	hard_assert(tcbs, 0);

	vm_t bottom = alloc_page(MM_O0, 0);
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
		p = t;
	}

	t->eid = t->pid;
	t->rid = p->rid;
	t->rpc.vmem = create_vmem();

	return t;
}

static stat_t __copy_proc(struct tcb *p, struct tcb *n)
{
	/** \todo Copy memory regions, and mark them MR_COW, as well as copy
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
		__copy_proc(p, n); /* we have a parent thread */

	return n;
}

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

struct tcb *cur_tcb()
{
	return cpu_tcb[cpu_id()];
}

struct tcb *cur_proc()
{
	struct tcb *t = cur_tcb();
	return get_tcb(t->eid);
}

void use_tcb(struct tcb *t)
{
	t->cpu_id = cpu_id();
	cpu_tcb[cpu_id()] = t;
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
