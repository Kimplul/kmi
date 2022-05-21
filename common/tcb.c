#include <apos/tcb.h>
#include <arch/cpu.h>
#include <apos/mem.h>
#include <apos/pmem.h>
#include <apos/nodes.h>
#include <apos/types.h>
#include <apos/assert.h>
#include <apos/string.h>
#include <arch/vmem.h>

/* arguably exessively many globals... */
static id_t start_tid;
static size_t num_tids;

static struct node_root root;
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
	destroy_nodes(&root);
	free_page(MM_O2, (pm_t)tcbs);
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

struct tcb *new_thread()
{
	if (unlikely(!tcbs))
		return 0;

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

	return t;
}

void destroy_thread(struct tcb *t)
{
	if (unlikely(!tcbs))
		return;

	/* remove thread id from list */
	tcbs[t->tid] = 0;

	/* free associated kernel stack */
	vm_t bottom = align_down((vm_t)t, __o_size(MM_O0));
	free_page(MM_O0, (pm_t)bottom);
}

struct tcb *cur_tcb()
{
	return cpu_tcb[cpu_id()];
}

void use_tcb(struct tcb *t)
{
	cpu_tcb[cpu_id()] = t;
}

struct tcb *get_tcb(id_t tid)
{
	if (unlikely(!tcbs))
		return 0;

	return tcbs[tid];
}

stat_t clone_tcb_maps(struct tcb *r)
{
	hard_assert(r && !r->parent, ERR_INVAL);
	struct tcb *t = r;
	while ((t = t->next)) {
		stat_t ret = clone_vmbranch(r->b_r, t->b_r);
		if (ret)
			return ret;
	}

	return OK;
}
