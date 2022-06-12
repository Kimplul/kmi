/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_RISCV_TYPES_H
#define APOS_RISCV_TYPES_H

/**
 * @file types.h
 * riscv64 definitions of arch-specific data types, possibly made redundant by
 * \ref include/apos/types.h but kept around just to be sure.
 */

#include <apos/types.h>

/** Virtual memory address. */
typedef uintptr_t vm_t;

/** Physical memory address. */
typedef uintptr_t pm_t;

#endif /* APOS_RISCV_TYPES_H */
