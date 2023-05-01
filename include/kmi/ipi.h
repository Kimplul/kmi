/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023 Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_IPI_H
#define KMI_IPI_H

/**
 * @file ipi.h
 *
 * IPI function definitions.
 */

#include <kmi/types.h>
#include <kmi/uapi.h>
#include <kmi/tcb.h>

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

#endif /* KMI_IPI_H */
