#include <apos/tcb.h>
#include <arch/cpu.h>
#include <apos/mem.h>
#include <apos/pmem.h>
#include <apos/nodes.h>
#include <apos/types.h>
#include <apos/string.h>

/* arguably exessively many globals... */
static id_t start_tid;
static size_t num_tids;

static struct node_root root;
static struct tcb **tcbs;

/* if we ever support systems with massive amounts of cpus, this should probably
 * be allocated at runtime */
static struct tcb *cpu_tcb[MAX_CPUS] = {0};

void init_tcbs()
{
	/* assumption: init_tcb called after memory subsystem is initialized */
	init_nodes(&root, sizeof(struct tcb));
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

	struct tcb *t = (struct tcb *)get_node(&root);
	t->tid = __alloc_tid(t);
	return t;
}

void destroy_thread(struct tcb *t)
{
	if (unlikely(!tcbs))
		return;

	/* remove thread id from list */
	tcbs[t->tid] = 0;
	/* free node associated with tcb */
	free_node(&root, t);
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
