#ifndef APOS_TCB_H
#define APOS_TCB_H

#include <apos/mem_regions.h>
#include <apos/types.h>
#include <tcb.h> /* arch-specific data */

struct tcb {
	struct arch_tcbd tcbd;

	/* mapping data
	 * TODO: should mem_region_root be renamed mem_root or something? feels
	 * kind of clunky */
	struct mem_region_root sp_r;

	id_t tid;

	vm_t callback;

	vm_t proc_stack;
	vm_t proc_stack_top;

	vm_t call_stack;
	vm_t call_stack_top;

	/* entry point */
	vm_t entry;

	/* vm root branch */
	struct vm_branch *b_r;

	struct tcb *parent;
	struct tcb *next;
};

void init_tcbs();
void destroy_tcbs();

struct tcb *new_thread();
void destroy_thread(struct tcb *);

struct tcb *cur_tcb();
void use_tcb(struct tcb *);

struct tcb *get_tcb(id_t tid);

stat_t clone_tcb_maps(struct tcb *);

#endif /* APOS_TCB_H */
