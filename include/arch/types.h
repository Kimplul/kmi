/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_ARCH_TYPES_H
#define APOS_ARCH_TYPES_H

/**
 * @file types.h
 * Arch-specific types.
 *
 * Doxygen thinks types.h if include/apos/types.h, but it's actually
 * arch/whatever/include/types.h. Not sure how to fix that.
 *
 * \todo Fix incorrect include file reference in doxygen.
 */

#if defined(riscv64)
#include "../../arch/riscv64/include/types.h"
#elif defined(riscv32)
#include "../../arch/riscv64/include/types.h"
#endif

#endif /* APOS_ARCH_TYPES_H */
