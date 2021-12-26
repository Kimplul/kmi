#ifndef APOS_PROC_H
#define APOS_PROC_H

#include <apos/tcb.h>
void jump_to_userspace(struct tcb *t, vm_t bin, int argc, char **argv);

#endif /* APOS_PROC_H */
