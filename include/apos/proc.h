#ifndef APOS_PROC_H
#define APOS_PROC_H

#include <apos/tcb.h>
#include <apos/vmem.h>

void jump_to_userspace(struct tcb *t, int argc, char **argv);
void init_proc(void *fdt, struct vm_branch *b);

#endif /* APOS_PROC_H */
