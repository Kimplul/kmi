#ifndef APOS_ARCH_PROC_H
#define APOS_ARCH_PROC_H

/**
 * @file proc.h
 * Arch-specific process related stuff.
 */

#if defined(riscv64)
#include "../../arch/riscv64/include/proc.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/proc.h"
#endif

stat_t set_return(vm_t r);
/*TODO: should this be in arch/tcb.h or something? */
stat_t prepare_thread(struct tcb *t);
stat_t run_init(struct tcb *t, void *fdt);

#endif /* APOS_ARCH_PROC_H */
