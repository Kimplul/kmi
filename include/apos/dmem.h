/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_DEV_H
#define APOS_DEV_H

/**
 * @file dmem.h
 * Device memory handling, i.e. anything outside of RAM.
 * \todo Make global parameters functions instead.
 */

#include <apos/types.h>
#include <apos/vmem.h>

/** Provides access to the global parameter. \global */
extern pm_t __pre_base;

/** Provides access to the global parameter. \global */
extern pm_t __pre_top;

/** Provides access to the global parameter. \global */
extern pm_t __post_base;

/** Provides access to the global parameter. \global */
extern pm_t __post_top;

/**
 * Initialize device memory.
 * @note Currently I assume there is only one RAM region. This may not be the
 * case with NUMA.
 *
 * @param ram_base Start of RAM.
 * @param ram_top Top of RAM.
 * @return stat_t \ref OK on success, nothing else at the moment.
 */
stat_t init_devmem(pm_t ram_base, pm_t ram_top);

/**
 * Allocate direct device mapping.
 * @note Returned address might not be the same as the requested address. The
 * caller is responsible for passing the returned address to \ref free_devmem(), not
 * the requested address.
 *
 * @param p Process to allocate device mapping to.
 * @param dev_start Start of physical address where device is mapped to.
 * @param bytes Minimum size of direct memory allocation.
 * @param flags Access flags of allocation.
 * @return Address to start of allocation if succesful, \c 0 otherwise.
 */
vm_t alloc_devmem(struct tcb *p, pm_t dev_start, size_t bytes, vmflags_t flags);

/**
 * Free direct device mapping.
 *
 * @param p Process to free mapping from.
 * @param dev_start Start of allocation.
 * @return \ref OK when succesful, \c ERR_NF when mapping not found.
 */
stat_t free_devmem(struct tcb *p, vm_t dev_start);

#endif /* APOS_DEV_H */
