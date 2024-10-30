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

/**
 * Initialize Big Kernel Lock.
 * In theory, the BKL being statically initialized to 0 should be enough, but
 * this feels a bit safer.
 */
void bkl_init();

/** Lock the Big Kernel Lock. */
void bkl_lock();

/** Unlock the Big Kernel Lock. */
void bkl_unlock();

#endif /* KMI_BKL_H */
