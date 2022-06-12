/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_RISCV_LOCK_H
#define APOS_RISCV_LOCK_H
/**
 * @file lock.h
 * riscv64 implementation of arch-specific lock behaviour, currently only \ref
 * optional_pause().
 */

/**
 * riscv64 does not have a way to inform the cpu of a spinlock (at least at the
 * moment, it may be added in as a hint later on)
 */
#define optional_pause()

#endif /* APOS_RISCV_LOCK_H */
