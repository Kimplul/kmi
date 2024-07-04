/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2024, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_ORPHANAGE_H
#define KMI_OPRHANAGE_H

/**
 * @file orphanage.h
 *
 * Stuff related to orphaned threads.
 *
 * Oprhaned threads are threads whose owning process has been destroyed.
 * They are always assigned to the init process, which will generally destroy
 * them whenever it gets a chance.
 */

#include <kmi/tcb.h>

/**
 * @param t Thread suspected of being an orphan.
 * @return \ref true if \p t is an orphan, \ref false otherwise.
 */
bool orphan(struct tcb *t);

/**
 * Mark \p t orphaned.
 *
 * @param t Thread to orphanize.
 */
void orphanize(struct tcb *t);

/**
 * Assign \p t to the init process and jump to it.
 * \p t must be an orphan!
 * \p t must be in the process of swapping to its root process, either by
 * returning from an rpc or being swapped to.
 *
 * @param t Orphaned thread.
 */
__noreturn void unorphanize(struct tcb *t);

#endif /* KMI_OPRHANAGE_H */
