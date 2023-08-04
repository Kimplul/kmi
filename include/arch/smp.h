/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file smp.h
 * Arch specific multicore bringup stuff.
 */

#ifndef KMI_ARCH_SMP_H
#define KMI_ARCH_SMP_H

#include <kmi/vmem.h>

/**
 * Bring up other cores in system.
 *
 * @param b Direct + kernel mappin virtual memory to use.
 * @param fdt Flattened device tree.
 */
void smp_bringup(struct vmem *b, void *fdt);

#endif /* KMI_ARCH_SMP_H */
