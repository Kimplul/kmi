#ifndef APOS_PROC_H
#define APOS_PROC_H

#include <apos/tcb.h>
void jump_to_userspace(struct tcb *t, char **argv, int argc);

#endif /* APOS_PROC_H */
