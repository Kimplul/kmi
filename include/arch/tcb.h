#ifndef APOS_ARCH_TCB_H
#define APOS_ARCH_TCB_H

/**
 * @file lock.h
 * Arch-specific tcb stuff.
 */

#if defined(riscv64)
#include "../../arch/riscv64/include/tcb.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/tcb.h"
#endif

#endif /* APOS_ARCH_TCB_H */
