/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_RISCV_TYPES_H
#define KMI_RISCV_TYPES_H

/**
 * @file types.h
 * riscv64 definitions of arch-specific data types, possibly made redundant by
 * \ref include/kmi/types.h but kept around just to be sure.
 */

#include <kmi/types.h>

/** Virtual memory address. */
typedef uintptr_t vm_t;

/** Physical memory address. */
typedef uintptr_t pm_t;

#endif /* KMI_RISCV_TYPES_H */
