/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_INITRD_H
#define KMI_INITRD_H

/**
 * @file initrd.h
 * Initrd handling.
 */

#include <kmi/types.h>
#include <kmi/pmem.h>
#include <kmi/vmem.h>

/**
 * Get \c init program size in bytes.
 *
 * @param fdt Global FDT pointer.
 * @return Size of \c init in bytes.
 */
size_t get_init_size(const void *fdt);

/**
 * Get \c init program base address.
 *
 * @param fdt Global FDT pointer.
 * @return Start of \c init.
 */
vm_t get_init_base(const void *fdt);

/**
 * Get \c initrd top address.
 *
 * @param fdt Global FDT pointer.
 * @return Top address of \c initrd.
 */
pm_t get_initrdtop(const void *fdt);

/**
 * Get \c initrd base address.
 *
 * @param fdt Global FDT pointer.
 * @return Base address of \c initrd.
 */
pm_t get_initrdbase(const void *fdt);

/**
 * Get size of initrd in bytes.
 *
 * @param fdt Global FDT pointer.
 * @return Size of \c initrd.
 */
size_t get_initrdsize(const void *fdt);

/**
 * Move \c init program to some other region in memory.
 *
 * @param fdt Global FDT pointer.
 * @param target Where to move to.
 * @return OK on success.
 */
stat_t move_init(const void *fdt, void *target);

#endif /* KMI_INITRD_H */
