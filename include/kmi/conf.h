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
 * @remark I might implement per-thread stack sizes at some point.
 *
 * @return Size of a regular thread stack, primarily just used to pass to \ref
 * alloc_stack().
 */
size_t thread_stack_size();

/**
 * Provides access to the runtime global parameter. This sets the maximum size
 * a single rpc stack instance can be.
 *
 * @return Size of one RPC stack entry, generally a single RPC shouldn't take
 * up all of the available RPC stack space so we limit how much each call is
 * allowed at a maximum, effectively enforcing a minimum number of RPC calls
 * that a thread must be allowed to execute.
 *
 * I feel like this description is overly complicated but I can't think of a way
 * to say it more clearly at the moment.
 */
size_t rpc_stack_size();

#endif /* KMI_CONF_H */
