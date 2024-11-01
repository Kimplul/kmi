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

/** Maximum number of concurrently active threads. */
static id_t num_tcbs;

/** Pointer to array of \ref tcb structures. Length of the array is \c num_tids.*/
static struct tcb **tcbs;

/**
 * Array of thread control block associated with each cpu.
 *
 * \todo If we ever support systems with massive amounts of cpus, this should probably
 * be allocated at runtime.
 */
static struct tcb *__cpu_tcb[MAX_CPUS] = { 0 };

size_t max_tcbs()
{
	return num_tcbs;
}

void init_tcbs()
{
	/* MM_O1 is 2MiB on riscv64, so 262144 different possible thread ids.
	 * Should be enough, if we're really strapped for memory I might try
	 * something smaller but this is fine for now. */
	tcbs = (struct tcb **)alloc_page(MM_O1);
	num_tcbs = order_size(MM_O1) / sizeof(struct tcb *);
	assert(is_powerof2(num_tcbs));
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

		if (tcbs[i & (num_tcbs - 1)] || i == 0)
			continue;

		tcbs[i & (num_tcbs - 1)] = t;
		start_tid = i + 1;
		return i;
	}

	return ERR_NF;
}

/**
 * Initialize thread that isn't bound to any parent process, i.e. it will become
 * a process in itself.
 *
 * @param t Partially constructed tcb to construct further.
 * @return OK/ERR_OOMEM.
 */
static stat_t __init_free_thread(struct tcb *t)
{
	if (!(t->proc.vmem = create_vmem()))
		return ERR_OOMEM;

	if (init_uvmem(t)) {
		destroy_vmem(t->proc.vmem);
		return ERR_OOMEM;
	}

	if (!(t->rpc.vmem = create_vmem())) {
		destroy_vmem(t->proc.vmem);
		return ERR_OOMEM;
	}

	if (setup_rpc_stack(t)) {
		destroy_rpcmem(t->rpc.vmem);
		destroy_vmem(t->proc.vmem);
		return ERR_OOMEM;
	}

	t->pid = t->tid;
	t->eid = t->tid;
	t->rid = t->tid;
	/* callback is set later */
	return OK;
}

/**
 * Initialize thread that is bound to some parent process.
 *
 * @param p Parent tcb.
 * @param t Partially constructed tcb to construct further.
 * @return OK/ERR_OOMEM.
 */
static stat_t __init_owned_thread(struct tcb *p, struct tcb *t)
{
	t->eid = p->rid;
	t->pid = p->rid;
	t->rid = p->rid;

	/* someone else owns our vmem */
	t->proc.vmem = NULL;
	t->callback = p->callback;

	if (!(t->rpc.vmem = create_vmem()))
		return NULL;

	if (setup_rpc_stack(t)) {
		destroy_rpcmem(t->rpc.vmem);
		return ERR_OOMEM;
	}

	t->regs = t->rpc_stack - sizeof(struct call_ctx);
	reference_thread(p);
	return OK;
}

struct tcb *create_thread(struct tcb *p)
{
	assert(tcbs);

	vm_t bottom = alloc_page(KERNEL_STACK_PAGE_ORDER);
	if (!bottom)
		return NULL;

	/* move tcb to top of kernel stack, keeping alignment in check
	 * (hopefully) */
	/** \todo check alignment */
	vm_t top = bottom + order_size(MM_O0) - sizeof(struct tcb);
	struct tcb *t = (struct tcb *)align_down(top, sizeof(long));
	memset(t, 0, sizeof(struct tcb));

	id_t tid = __alloc_tid(t);
	tcbs[tid] = t;
	t->tid = tid;
	t->state = 0;

	stat_t r = OK;
	if (p) r = __init_owned_thread(p, t);
	else r = __init_free_thread(t);

	if (r) {
		free_page(MM_O0, bottom);
		return NULL;
	}


	struct tcb *parent = get_rproc(t);
	clone_uvmem(parent->proc.vmem, t->rpc.vmem);

	reference_thread(t);
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
	n->exec = p->exec;
	n->callback = p->callback;

	copy_regs(n, p);
	copy_caps(n->caps, p->caps);
	copy_rpc_stack(p, n);
	return copy_uvmem(n, p);
}

struct tcb *create_proc(struct tcb *p)
{
	assert(tcbs);

	/* create a new thread outside the current process */
	struct tcb *n = create_thread(NULL);
	if (!n)
		return NULL;

	if (p)
		__copy_proc(p, n); /* we have a parent process i.e. fork */

	return n;
}

/**
 * Destroy data associated with thread.
 *
 * @param t Thread whose data to destroy.
 */
static void __destroy_thread_data(struct tcb *t)
{
	assert(t->refcount == 0);
	assert(zombie(t));

	/* remove ourselves from the thread pool */
	tcbs[t->tid] = 0;

	/* forcefully free last struggling bits of memory, assuming we own the
	 * uvmem */
	destroy_uvmem(t);

	/* free associated kernel stack and the structure itself */
	vm_t bottom = align_down((vm_t)t, order_size(MM_O0));
	free_page(MM_O0, (pm_t)bottom);
}

stat_t destroy_thread(struct tcb *t)
{
	assert(tcbs);
	assert(t->tid != 1);

	/* remove reference to root process */
	struct tcb *r = get_rproc(t);
	if (r != t)
		/* if we're our own root process, we unreference ourselves later */
		unreference_thread(r);

	/* free memory backing rpc stack */
	destroy_rpc_stack(t);
	destroy_rpcmem(t->rpc.vmem);

	unqueue_ipi(t);

	/** @todo timers, irqs? theoretically we could allow them to stay and
	 * let the handler check if the thread is still interested in the
	 * interrupt */

	/* mark us as zombies */
	set_bits(t->state, TCB_ZOMBIE);
	t->rid = 0;

	unreference_thread(t);
	return OK;
}

stat_t destroy_proc(struct tcb *p)
{
	assert(tcbs);
	assert(is_proc(p));

	/* clear all privately owned memory regions, keep shared ones alive for
	 * now */
	clear_uvmem(p);

	/* don't destroy thread data just yet, let the thread destroy itself
	 * later */
	return OK;
}

void reference_thread(struct tcb *t)
{
	if (!t)
		return;

	t->refcount++;
}

void unreference_thread(struct tcb *t)
{
	if (!t)
		return;

	t->refcount--;
	if (t->refcount == 0)
		__destroy_thread_data(t);

	assert(t->refcount >= 0);
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

	use_vmem(t->rpc.vmem);
}

struct tcb *get_tcb(id_t tid)
{
	assert(tcbs);

	if (tid <= 0)
		return NULL;

	struct tcb *t = tcbs[tid & (num_tcbs - 1)];
	if (!t)
		return NULL;

	if (t->tid != tid)
		return NULL;

	return t;
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
