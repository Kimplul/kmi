/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2024, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_NOTIFY_H
#define KMI_NOTIFY_H

/**
 * @file notify.h
 *
 * Notification handling stuff.
 */

#include <kmi/syscalls.h>
#include <kmi/tcb.h>

/**
 * Try to send a notification to a thread. Sets up a new rpc call, should be
 * returned from with \ref ipc_ghost() to restore register state.
 *
 * If the thread is idle, immediately swap to it.
 *
 * If the thread is doing an rpc, queue the notification to be executed when the
 * thread returns to the parent process.
 *
 * If the thread is currently executing on another core, send an ipi to
 * the core to switch threads.
 *
 * Implemented in uapi/ipc.c.
 *
 * @param t Thread to send notification to.
 * @param flag Which type of notification to send. If 0, sends out queued
 * notifications if there are any, otherwise just returns.
 */
void notify(struct tcb *t, enum notify_flag flag);

#endif /* KMI_NOTIFY_H */
