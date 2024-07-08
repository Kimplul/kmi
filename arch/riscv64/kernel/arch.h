/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file arch.h
 *
 * Random tools common to riscv.
 */

#ifndef KMI_RISCV_ARCH_H
#define KMI_RISCV_ARCH_H

#include <kmi/vmem.h>

/**
 * Map cpu id to hart id.
 * Defined as a macro to allow for stuff like
 * @code
 * cpuid_to_hartid(0) = 20;
 * @endcode
 *
 * @param x Hart ID.
 * @return Corresponding CPU ID.
 */
id_t cpuid_to_hartid(id_t x);

/**
 * Create mapping between \p cpuid and \p hartid.
 *
 * @param cpuid CPU ID to map to \p hartid.
 * @param hartid Hart ID to map to \p CPU ID.
 */
void set_hartid(id_t cpuid, id_t hartid);

/**
 * Find the cpu id that corresponds to hart id.
 *
 * @param hart Hart to find corresponding cpu id for.
 * @return Corresponding cpu id. 0 if not found, though this should maybe be a
 * panic situation. Currently just sets the core to sleep.
 */
id_t hartid_to_cpuid(id_t hart);

/**
 * Format branch and mode information into something that can be written to
 * SATP.
 *
 * @param branch Top branch of virtual memory.
 * @param mode Mode to use. Sv32/Sv39/etc.
 * @return Corresponding SATP value.
 */
pm_t branch_to_satp(struct vmem *branch, enum mm_mode mode);

#endif /* KMI_RISCV_ARCH_H */
