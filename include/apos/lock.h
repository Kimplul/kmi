/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_LOCK_H
#define APOS_LOCK_H

/**
 * @file lock.h
 * Atomic locks, currently only \ref spin_lock() and \ref spin_unlock(). Mutex is
 * probably overkill for this project.
 */

#include <apos/atomic.h>
#include <apos/irq.h>

/**
 * Typedef for atomic_int.
 *
 * In apos, spinlocks are implemented with compiler-intrinsic atomic integers,
 * that essentially just contain some flags. Currently all spinlocks
 * enable/disable irqs.
 *
 * \todo irq contexts?
 */
typedef atomic_int spinlock_t;

#include <arch/lock.h>

/**
 * Lock a spinlock.
 *
 * @param lck Pointer to lock.
 */
static inline void spin_lock(spinlock_t *lck)
{
	disable_irq();
	do {
		while (atomic_load_explicit(lck, memory_order_acquire))
			optional_pause();

	} while (atomic_exchange_explicit(lck, 1, memory_order_acq_rel));
}

/**
 * Unlock a spinlock.
 *
 * @param lck Pointer to lock.
 */
static inline void spin_unlock(spinlock_t *lck)
{
	atomic_store_explicit(lck, 0, memory_order_release);
	enable_irq();
}

#endif /* APOS_LOCK_H */
