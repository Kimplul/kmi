/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023 Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_CANARY_H
#define KMI_CANARY_H

/**
 * @file canary.h
 * Kernel stack canary handling.
 */

#include <kmi/tcb.h>
#include <kmi/types.h>

/**
 * Place canary at end of kernel stack.
 *
 * @param t \ref tcb whose kernel stack to place canary in.
 */
void set_canary(struct tcb *t);

/**
 * Check that canary hasn't been accidentally overwritten.
 *
 * @param t \ref tcb whose kernel stack canary to check.
 * @return \ref true if overwritten, \ref false otherwise.
 */
bool check_canary(struct tcb *t);

#endif /* KMI_CANARY_H */
