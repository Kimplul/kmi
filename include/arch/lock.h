/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_ARCH_LOCK_H
#define APOS_ARCH_LOCK_H

/**
 * @file lock.h
 * Arch-specific lock stuff.
 */

#if defined(riscv64)
#include "../../arch/riscv64/include/lock.h"
#if defined(riscv32)
#include "../../arch/riscv32/include/lock.h"
#endif

#endif /* APOS_ARCH_LOCK_H */
