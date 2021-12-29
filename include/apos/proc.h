#ifndef APOS_PROC_H
#define APOS_PROC_H

#include <apos/tcb.h>
#include <apos/vmem.h>

void jump_to_userspace(struct tcb *t, vm_t bin, int argc, char **argv);
vm_t setup_call_stack(struct tcb *t, vm_t start, size_t bytes);

#endif /* APOS_PROC_H */
