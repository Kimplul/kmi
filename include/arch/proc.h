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
stat_t set_ipc(struct tcb *t, id_t pid, id_t tid);
/*\todo: should this be in arch/tcb.h or something? */
stat_t set_thread(struct tcb *t, vm_t stack);
stat_t run_init(struct tcb *t, void *fdt);

#endif /* APOS_ARCH_PROC_H */
