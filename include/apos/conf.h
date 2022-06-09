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
 * \see __thread_stack_size.
 * \global
 * \todo This should probably also be a function instead.
 */
extern size_t __call_stack_size;

#endif /* APOS_CONF_H */
