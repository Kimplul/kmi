/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_PROC_H
#define KMI_PROC_H

/**
 * @file proc.h
 * Process handling subsystem, should likely be merged into \ref
 * include/kmi/tcb.h.
 */

#include <kmi/tcb.h>
#include <kmi/vmem.h>

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

#endif /* KMI_PROC_H */
