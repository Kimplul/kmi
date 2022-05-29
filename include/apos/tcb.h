#ifndef APOS_TCB_H
#define APOS_TCB_H

/**
 * @file tcb.h
 * Process/thread handling.
 */

#include <apos/mem_regions.h>
#include <apos/types.h>
#include <arch/tcb.h> /* arch-specific data */

/* process(/main) threads don't have any previous threads */
#define is_proc(t) (t->rid == t->tid)
#define is_rpc(t) (t->rid == t->pid)

#define get_proc(t) (get_tcb(t->eid))
#define get_rproc(t) (get_tcb(t->rid))

/* forward declaration */
struct tcb;

struct tcb_ctx {
	struct vmem *vmem;
	struct tcb *next;
	struct tcb *prev;
};

struct tcb {
	struct arch_tcbd tcbd;

	/* mapping data */
	struct mem_region_root sp_r;

	id_t eid;
	id_t pid;

	id_t rid;
	id_t tid;

	vm_t callback;

	vm_t thread_stack;
	vm_t thread_stack_top;
	vm_t thread_storage;

	struct tcb_ctx proc;
	struct tcb_ctx rpc;
};

void init_tcbs();
void destroy_tcbs();

struct tcb *create_thread(struct tcb *p);
struct tcb *create_proc(struct tcb *p);
stat_t destroy_thread(struct tcb *t);
stat_t destroy_proc(struct tcb *p);

stat_t attach_rpc(struct tcb *r, struct tcb *t);
stat_t detach_rpc(struct tcb *r, struct tcb *t);
stat_t attach_proc(struct tcb *r, struct tcb *t);
stat_t detach_proc(struct tcb *r, struct tcb *t);

struct tcb *cur_tcb();
struct tcb *cur_proc();
void use_tcb(struct tcb *);

struct tcb *get_tcb(id_t tid);

stat_t clone_proc_maps(struct tcb *);
stat_t clone_rpc_maps(struct tcb *);
stat_t alloc_stacks(struct tcb *);

#endif /* APOS_TCB_H */
