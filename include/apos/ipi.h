#ifndef APOS_IPI_H
#define APOS_IPI_H

/**
 * @file ipi.h
 *
 * IPI function definitions.
 */

#include <apos/types.h>
#include <apos/uapi.h>
#include <apos/tcb.h>

/**
 * Clear potential IPI in \p t, and return its value.
 *
 * @param t \ref tcb to clear possible IPI status of.
 * @return \ref true if \p was interrupted by IPI, \ref false otherwise.
 */
bool clear_ipi(struct tcb *t);

/**
 * Send IPI to \p t. Assumes \c running(t).
 *
 * @param t \ref tcb to send IPI to.
 */
void send_ipi(struct tcb *t);

/**
 * Handle IPI.
 *
 * @param t Thread who was interrupted by IPI.
 * @return Possible arguments to IPI.
 */
struct sys_ret handle_ipi(struct tcb *t);

#endif /* APOS_IPI_H */
