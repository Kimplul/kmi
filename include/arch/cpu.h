#ifndef APOS_CPU_H
#define APOS_CPU_H

/**
 * @file cpu.h
 * Arch-specific cpu handling, generally implemented in
 * arch/whatever/kernel/cpu.c
 */

#include <apos/types.h>

#if defined(riscv64)
#include "../../arch/riscv64/include/cpu.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/cpu.h"
#endif

/**
 * Get current CPU ID.
 * ID must be in range \c 0 .. \c num_cpus, where \c num_cpus is the number of
 * cpus on this system.
 *
 * @return Current CPU ID.
 */
id_t cpu_id();

/* TODO: add more cpu handling functions */

#endif /* APOS_CPU_H */
