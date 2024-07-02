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
 * Send IPI to \p t. Assumes \c running(t).
 *
 * @param t \ref tcb to send IPI to.
 */
void send_ipi(struct tcb *t);

/** Handle IPI. */
void handle_ipi();

#endif /* KMI_IPI_H */
