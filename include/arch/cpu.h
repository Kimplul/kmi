#ifndef APOS_CPU_H
#define APOS_CPU_H

/**
 * @file cpu.h
 * Arch-specific cpu handling, generally implemented in
 * arch/whatever/kernel/cpu.c
 */

#include <apos/types.h>
#include <cpu.h>

id_t cpu_id();
/* TODO: add more cpu handling functions */

#endif /* APOS_CPU_H */
