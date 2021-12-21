#ifndef APOS_TCB_H
#define APOS_TCB_H

struct tcb;
struct sp_reg_root;

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

	id_t pid;
	id_t tid;

	vm_t stack;
	vm_t heap;
	vm_t bin;

	struct vm_branch_t *b_r;
	struct sp_reg_root sp_r;
};

void threads_insert(struct tcb *t);

#endif /* APOS_TCB_H */
