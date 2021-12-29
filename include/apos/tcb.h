#ifndef APOS_TCB_H
#define APOS_TCB_H

struct tcb;
struct sp_reg_root;

#include <tcb.h>
#include <vmem.h>
#include <apos/vmem.h>
#include <apos/types.h>
#include <apos/sp_tree.h>

typedef size_t id_t;

struct sp_reg_root {
	struct sp_root free_regions;
	struct sp_root used_regions;
};

struct tcb {
	struct sp_node sp_n;
	struct sp_reg_root sp_r;
	struct arch_tcbd tcbd;

	id_t pid;
	id_t tid;

	vm_t callback;
	vm_t proc_stack;
	vm_t call_stack;

	vm_t entry;

	struct vm_branch *b_r;
};

void threads_insert(struct tcb *t);
struct tcb *cur_tcb();
struct tcb *get_tcb(id_t tid);

#endif /* APOS_TCB_H */
