/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2024, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_BKL_H
#define KMI_BKL_H

/**
 * @file bkl.h
 *
 * Stuff for handling the Big Kernel Lock.
 */

#include <kmi/lock.h>

/** Big Kenrel Lock. */
extern spinlock_t bkl;

/** Lock the Big Kernel Lock. */
static inline void bkl_lock()
{
	spin_lock(&bkl);
}

/** Unlock the Big Kernel Lock. */
static inline void bkl_unlock()
{
	spin_unlock(&bkl);
}

#endif /* KMI_BKL_H */
