#ifndef APOS_CANARY_H
#define APOS_CANARY_H

/**
 * @file canary.h
 * Kernel stack canary handling.
 */

#include <apos/tcb.h>
#include <apos/types.h>

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

#endif /* APOS_CANARY_H */
