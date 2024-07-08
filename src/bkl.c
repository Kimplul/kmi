/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2024, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#include <kmi/lock.h>

/**
 * @file bkl.c
 *
 * Instanciation of the big kernel lock.
 */

/** The Big Kernel Lock. */
static spinlock_t bkl = 0;

void bkl_lock()
{
	spin_lock(&bkl);
}

void bkl_unlock()
{
	spin_unlock(&bkl);
}
