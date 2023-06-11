/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_CONF_H
#define KMI_CONF_H

/**
 * @file conf.h
 * Global configuration file, gives extern access to runtime configuration
 * parameters when they're implemented.
 */

#include <kmi/types.h>

/**
 * Provides access to the runtime global parameter.
 * \remark Note that runtime parameter passing is not yet implemented, and I might
 * implement per-thread stack sizes as well.
 * \global
 * \todo This should probably be a function instead.
 */
extern size_t __thread_stack_size;

/**
 * Provides access to the runtime global parameter.
 * Must be at most RPC_STACK_TOP - RPC_STACK_BASE.
 * Essentially, each thread gets allocated this many bytes of total stack space
 * that will be used during thread migrations. Each migration instance may at
 * most take up __rpc_stack_size bytes, and during a thread migration the
 * currently available free stack space is checked.
 *
 * Previous instances are unmapped, making them unaccessible to the current
 * instance.
 *
 * \see __thread_stack_size.
 * \global
 * \todo This should probably also be a function instead.
 */
extern size_t __call_stack_size;

/**
 * Provides access to the runtime global parameter. This sets the maximum size
 * a single rpc stack instance can be.
 *
 * \global
 */
extern size_t __rpc_stack_size;

#endif /* KMI_CONF_H */
