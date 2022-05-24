#ifndef APOS_CONF_H
#define APOS_CONF_H

/**
 * @file conf.h
 * Global configuration file, gives extern access to runtime configuration
 * parameters when they're implemented.
 */

#include <apos/types.h>

extern size_t __thread_stack_size;
extern size_t __call_stack_size;

#endif /* APOS_CONF_H */
