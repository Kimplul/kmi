#ifndef APOS_PROC_H
#define APOS_PROC_H

/**
 * @file proc.h
 * Process handling subsystem, should likely be merged into \ref
 * include/apos/tcb.h.
 */

#include <apos/tcb.h>
#include <apos/vmem.h>

/**
 * Set up binary. Currently only supports ELF, not sure if other formats should
 * be supported.
 *
 * @param t Thread space to set up process in.
 * @param bin Address of binary to load.
 * @param interp Optional interpreter, mainly for PIE ELF binaries.
 * @return \ref OK on success, \ref ERR_INVAL if loading binary failed.
 * \todo Handle out of memory better?
 */
stat_t prepare_proc(struct tcb *t, vm_t bin, vm_t interp);

/**
 * Initialize process handling subsystem and setup \c init program.
 *
 * @param fdt Global FDT pointer.
 * @return \ref ERR_OOMEM when out of memory, \ref ERR_INVAL if loading \c init
 * failed, \ref OK otherwise.
 */
stat_t init_proc(void *fdt);

#endif /* APOS_PROC_H */
