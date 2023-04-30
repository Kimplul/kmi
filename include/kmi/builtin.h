/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_BUILTIN_H
#define KMI_BUILTIN_H

/**
 * @file builtin.h
 * Defines __has_builtin, if the compiler doesn't support it.
 *
 * Technically we only support clang and gcc at the moment, and they both
 * support __has_builtin, but if some interesting compiler comes along then
 * we'll be golden.
 */

/**
 * Check whether compiler supports the builtin.
 * Note that this definition is only for when the compiler doesn't provide it
 * automatically.
 *
 * @param x Base name of builtin to check for.
 * @return \c 0 when not supported, non-zero when supported.
 */
#ifndef __has_builtin
#define __has_builtin(x) (0)
#endif

#endif /* KMI_BUILTIN_H */
