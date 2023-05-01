/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_ARCH_TYPES_H
#define KMI_ARCH_TYPES_H

/**
 * @file types.h
 * Arch-specific types.
 *
 * Doxygen thinks types.h if include/kmi/types.h, but it's actually
 * arch/whatever/include/types.h. Not sure how to fix that.
 *
 * \todo Fix incorrect include file reference in doxygen.
 */

#if defined(riscv64)
#include "../../arch/riscv64/include/types.h"
#elif defined(riscv32)
#include "../../arch/riscv32/include/types.h"
#endif

#endif /* KMI_ARCH_TYPES_H */
