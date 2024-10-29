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

void bkl_init();

/** Lock the Big Kernel Lock. */
void bkl_lock();

/** Unlock the Big Kernel Lock. */
void bkl_unlock();

#endif /* KMI_BKL_H */
