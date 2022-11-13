/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_CONF_H
#define APOS_CONF_H

/**
 * @file conf.h
 * Global configuration file, gives extern access to runtime configuration
 * parameters when they're implemented.
 */

#include <apos/types.h>

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
 * Must be at most a fourth of \ref __call_stack_size?
 * (that way we can maybe pretty quickly check that we're running out of memory
 * for stack stuff and can return an error about it)
 *
 * \global
 */
extern size_t __rpc_stack_size;

#endif /* APOS_CONF_H */
