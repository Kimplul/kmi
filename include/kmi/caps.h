/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023 Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_CAPS_H
#define KMI_CAPS_H

/**
 * @file caps.h
 * Capabilities of threads.
 * \todo The list of capabilities should maybe be placed in some other file so
 * as to easier extract it into userspace programs.
 *
 * @todo is it realistic to assume we will never need more than 32 capabilities?
 * If so, we can remove the offset nonsense.
 */

#include <kmi/bits.h>

/**
 * Set capabilities.
 *
 * @param x Capabilities to modify.
 * @param c Capabilities to set.
 */
#define set_caps(x, c) set_bits(x, c)

/**
 * Clear capabilities.
 *
 * @param x Capabilities to modify.
 * @param c Capabilities to set.
 */
#define clear_caps(x, c) clear_bits(x, c)

/**
 * Copy capabilities.
 *
 * @param x Capabilities to copy to.
 * @param y Capabilities to copy from.
 */
#define copy_caps(x, y) (x = y)

/**
 * Check if something has capability.
 *
 * @param x Capabilities to check in.
 * @param c Capability to check for.
 * @return \ref true if \p x has \p c, \ref false otherwise.
 */
#define has_cap(x, c) (x & c)

#endif /* KMI_CAPS_H */
