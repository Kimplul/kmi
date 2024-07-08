/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file tcb.c
 * Thread control block handling implementation.
 */

#include <kmi/tcb.h>
#include <kmi/ipi.h>
#include <kmi/mem.h>
#include <kmi/conf.h>
#include <kmi/pmem.h>
#include <kmi/vmem.h>
#include <kmi/nodes.h>
#include <kmi/types.h>
#include <kmi/assert.h>
#include <kmi/string.h>
#include <kmi/canary.h>

#include <arch/cpu.h>
#include <arch/vmem.h>
#include <arch/proc.h>

/* arguably exessively many globals... */
/** Thread ID to start looking from when allocating new ID. */
static id_t start_tid = 0;

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
	assert(is_powerof2(num_tids));
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
	for (id_t i = start_tid;; ++i) {
		if (i <= 0)
			i = 1;

		/* we're completely full */
		if (i == stop_tid)
			return ERR_NF;

		if (tcbs[i & (num_tids - 1)] || i == 0)
			continue;

		tcbs[i & (num_tids - 1)] = t;
		start_tid = i + 1;
		return i;
	}

	return ERR_NF;
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

	t->thread_stack = __setup_thread_stack(p, thread_stack_size());
	if (!t->thread_stack)
		return ERR_OOMEM;

	/** \todo this only allows for a global stack size, what if a user wants
	 * per thread stack sizes? I guess allocate them yourself in userspace
	 * or something? */
	t->thread_stack_size = thread_stack_size();
	return OK;
}

void free_stack(struct tcb *t)
{
	free_uvmem(t, t->thread_stack);
}

struct tcb *create_thread(struct tcb *p)
{
	assert(tcbs);

	vm_t bottom = alloc_page(KERNEL_STACK_PAGE_ORDER);
	/* move tcb to top of kernel stack, keeping alignment in check
	 * (hopefully) */
	/** \todo check alignment */
	bottom = bottom + order_size(MM_O0) - sizeof(struct tcb);
	struct tcb *t = (struct tcb *)align_down(bottom, sizeof(long));
	memset(t, 0, sizeof(struct tcb));

	id_t tid = __alloc_tid(t);
	tcbs[tid] = t;
	t->tid = tid;
	t->state = 0;

	if (likely(p)) {
		t->pid = p->pid;
		/** @todo I'm assuming two threads can share the same vmem
		 * structure, this works on riscv but in the event that other
		 * systems don't we can easily turn this into a clone_uvmem. */
		t->proc.vmem = p->proc.vmem;
	} else {
		t->proc.vmem = create_vmem();
		init_uvmem(t, UVMEM_START, UVMEM_END);
		t->pid = t->tid;
		t->rid = t->tid;
		p = t;
	}

	t->eid = t->pid;
	t->rid = p->rid;
	t->rpc.vmem = create_vmem();
	setup_rpc_stack(t);
	reference_proc(p);

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
	n->thread_stack_size = p->thread_stack_size;

	copy_regs(n, p);
	copy_caps(n->caps, p->caps);
	return copy_uvmem(n, p);
}

struct tcb *create_proc(struct tcb *p)
{
	assert(tcbs);

	/* create a new thread outside the current process */
	struct tcb *n = create_thread(NULL);
	if (!n)
		return 0;

	if (p)
		__copy_proc(p, n); /* we have a parent process i.e. fork */

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
	assert(t->refcount == 0);

	/* remove ourselves from the thread pool */
	/** @todo this should be at the top of the function, and be wrapped in
	 * some kind of lock that checks that nobody reads the value while we're
	 * setting it to zero. get_tcb() should accordingly increment the
	 * reference count atomically. Also, an unget_tcb() is needed to
	 * decrement the reference count I guess? if we didn't have the BKL that
	 * is
	 */
	tcbs[t->tid] = 0;

	/* forcefully free last struggling bits of memory, assuming we own the
	 * uvmem */
	destroy_uvmem(t);

	/* free associated kernel stack and the structure itself */
	vm_t bottom = align_down((vm_t)t, order_size(MM_O0));
	free_page(MM_O0, (pm_t)bottom);
	return OK;
}

stat_t destroy_thread(struct tcb *t)
{
	assert(tcbs);
	assert(!is_proc(t));

	/* mark us as zombies */
	set_bits(t->state, TCB_ZOMBIE);
	t->rid = 0;

	/* remove reference to root process */
	unreference_proc(get_rproc(t));

	free_stack(t);

	/* free memory backing rpc stack */
	destroy_rpc_stack(t);

	/* free rpc vmem */
	destroy_vmem(t->rpc.vmem);

	unqueue_ipi(t);

	/** @todo timers, irqs? theoretically we could allow them to stay and
	 * let the handler check if the thread is still interested in the
	 * interrupt */

	/* someone still relies on us existing, don't actually free thread data
	 * quite yet */
	if (t->refcount)
		return OK;

	return __destroy_thread_data(t);
}

stat_t destroy_proc(struct tcb *p)
{
	assert(tcbs);
	assert(is_proc(p));

	/** @todo currently we don't care who else is in the address space when
	 * we start freeing stuff, one fairly simple way to deal with this is to
	 * just not care. A thread that tries to access some bit of freed memory
	 * will cause a segfault (eventually at least), and we can just check in
	 * the segfault handler if the thread has become orphaned.
	 * Currently no segfault handler exists, though. */

	set_bits(p->state, TCB_ZOMBIE);

	/* clear all privately owned memory regions, keep shared ones alive for
	 * now */
	clear_uvmem(p);

	/* don't destroy thread data just yet, let the thread destroy itself
	 * later */
	return OK;
}

void reference_proc(struct tcb *p)
{
	if (!p)
		return;

	assert(is_proc(p));
	p->refcount++;
}

void unreference_proc(struct tcb *p)
{
	if (!p)
		return;

	assert(is_proc(p));
	p->refcount--;
	if (zombie(p) && p->refcount == 0) {
		dbg("thread %ld is completely destroyed\n", (long)p->tid);
		__destroy_thread_data(p);
	}
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
	assert(tcbs);

	if (tid <= 0)
		return NULL;

	return tcbs[tid & (num_tids - 1)];
}

void set_return(struct tcb *t, vm_t v)
{
	t->exec = v;
}

bool running(struct tcb *t)
{
	return cpu_tcb(t->cpu_id) == t;
}

bool zombie(struct tcb *t)
{
	/* we shouldn't see any NULLs but they're effectively the same thing */
	if (!t)
		return true;

	/* thread doesn't belong to any process, a zombie */
	return t->rid == 0;
}
