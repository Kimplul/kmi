#ifndef APOS_ARCH_H
#define APOS_ARCH_H

/**
 * @file arch.h
 * Arch-specific generic stuff, generally implemented in
 * arch/whatever/kernel/main.c
 */

#include <apos/types.h>

stat_t setup_arch(void *fdt);

#endif /* APOS_ARCH_H */
