#ifndef APOS_BUILTIN_H
#define APOS_BUILTIN_H

/**
 * @file builtin.h
 * Defines __has_builtin, if the compiler doesn't support it.
 *
 * Technically we only support clang and gcc at the moment, and they both
 * support __has_builtin, but if some interesting compiler comes along then
 * we'll be golden.
 */

#ifndef __has_builtin
#define __has_builtin(x) (0)
#endif

#endif /* APOS_BUILTIN_H */
