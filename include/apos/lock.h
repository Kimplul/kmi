#ifndef LOCK_H
#define LOCK_H

#include <apos/atomic.h>
typedef atomic_int spinlock_t;

#include <lock.h>

static inline void spin_lock(spinlock_t *lck)
{
	do {
		while (atomic_load_explicit(lck, memory_order_acquire))
			optional_pause();

	} while (atomic_exchange_explicit(lck, 1, memory_order_acq_rel));
}

static inline void spin_unlock(spinlock_t *lck)
{
	atomic_store_explicit(lck, 0, memory_order_release);
}

#endif /* LOCK_H */
