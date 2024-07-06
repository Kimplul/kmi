/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2024, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_RISCV_UAPI_H
#define KMI_RISCV_UAPI_H

/**
 * @file uapi.h
 *
 * RISCV-specific stuff that should be visible to userspace, currently mainly
 * VM_R/VM_W/VM_X flags.
 */

/** Page is active. */
#define VM_V (1 << 0)

/** Page is readable. */
#define VM_R (1 << 1)

/** Page is writable. */
#define VM_W (1 << 2)

/** Page is executable. */
#define VM_X (1 << 3)

/** Page is user-accessible. */
#define VM_U (1 << 4)

/** Page is global. */
#define VM_G (1 << 5)

/** Page has been accessed. */
#define VM_A (1 << 6)

/** Page is dirty. */
#define VM_D (1 << 7)

#endif /* KMI_RISCV_UAPI_H */
