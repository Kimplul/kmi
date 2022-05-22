#ifndef APOS_PROC_H
#define APOS_PROC_H

/**
 * @file proc.h
 * Process handling subsystem, should likely be merged into \ref
 * include/apos/tcb.h.
 */

#include <apos/tcb.h>
#include <apos/vmem.h>

stat_t jump_to_userspace(struct tcb *t, int argc, char **argv);
stat_t init_proc(void *fdt, struct vmem *b);

#endif /* APOS_PROC_H */
