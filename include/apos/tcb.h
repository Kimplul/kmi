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
#define is_proc(t) (!t->prev)

struct tcb {
	struct arch_tcbd tcbd;

	/* mapping data
	 * TODO: should mem_region_root be renamed mem_root or something? feels
	 * kind of clunky */
	union {
		/* if we're the main thread, we control the memory regions */
		struct mem_region_root sp_r;
		/* if we're not the main thread, we have a pointer to the main
		 * thread */
		struct tcb *proc;
	};

	id_t pid;
	id_t tid;

	vm_t callback;

	vm_t proc_stack;
	vm_t proc_stack_top;

	vm_t call_stack;
	vm_t call_stack_top;

	/* entry point */
	vm_t entry;

	/* vm root branch */
	struct vmem *b_r;

	/* linked list of threads in this process */
	struct tcb *next;
	struct tcb *prev;
};

void init_tcbs();
void destroy_tcbs();

struct tcb *create_thread(struct tcb *p);
struct tcb *create_proc(struct tcb *p);
stat_t destroy_thread(struct tcb *t);
stat_t destroy_proc(struct tcb *p);

struct tcb *cur_tcb();
void use_tcb(struct tcb *);

struct tcb *get_tcb(id_t tid);

stat_t clone_tcb_maps(struct tcb *);

#endif /* APOS_TCB_H */
