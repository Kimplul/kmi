#ifndef APOS_PROC_H
#define APOS_PROC_H

/**
 * @file proc.h
 * Process handling subsystem, should likely be merged into \ref
 * include/apos/tcb.h.
 */

#include <apos/tcb.h>
#include <apos/vmem.h>

stat_t prepare_proc(struct tcb *t, vm_t bin, vm_t interp);
stat_t init_proc(void *fdt);

#endif /* APOS_PROC_H */
