/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_LOCK_H
#define KMI_LOCK_H

/**
 * @file lock.h
 * Atomic locks, currently only \ref spin_lock() and \ref spin_unlock(). Mutex is
 * probably overkill for this project.
 */

#include <kmi/atomic.h>
#include <kmi/irq.h>

/**
 * Typedef for atomic_int.
 *
 * In kmi, spinlocks are implemented with compiler-intrinsic atomic integers,
 * that essentially just contain some flags. Currently all spinlocks
 * enable/disable irqs.
 *
 * \todo irq contexts?
 */
typedef int spinlock_t;

#include <arch/lock.h>

/**
 * Lock a spinlock.
 *
 * @param lck Pointer to lock.
 */
static inline void spin_lock(spinlock_t *lck)
{
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
}

#endif /* KMI_LOCK_H */
