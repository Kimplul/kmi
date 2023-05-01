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

/** Helper typedef for capabilities. */
typedef unsigned char capflags_t;

enum {
	/** Thread is allowed to set capabilities of other threads. */
	CAP_CAPS = (1 << 0),

	/** Thread is allowed to modify process statuses, create/exec/fork/etc. */
	CAP_PROC = (1 << 1),

	/** Thread is allowed to force interrupt to callback in other thread. */
	CAP_CALL = (1 << 2),

	/** Thread is allowed to shut down system. */
	CAP_POWER = (1 << 3),

	/** Thread is allowed to access configuration parameters. */
	CAP_CONF = (1 << 4),
};

/**
 * Check that offset is OK.
 *
 * @param o Offset to check.
 * @return \ref true is OK, \ref false otherwise.
 */
#define cap_off_ok(o) (o == 0)

/**
 * Set capabilities.
 *
 * @param x Capabilities to modify.
 * @param o Offset.
 * @param c Capabilities to set.
 */
#define set_caps(x, o, c) set_bits(x, c)

/**
 * Clear capabilities.
 *
 * @param x Capabilities to modify.
 * @param o Offset.
 * @param c Capabilities to set.
 */
#define clear_caps(x, o, c) clear_bits(x, c)

/**
 * Copy capabilities.
 *
 * @param x Capabilities to copy to.
 * @param y Capabilities to copy from.
 */
#define copy_caps(x, y) (x = y)

/**
 * Get capabilities at offset \p o.
 *
 * @param x Capabilities to get from.
 * @param o Offset to get capabilities from.
 * @return Capabilities at offset \p o.
 */
#define get_caps(x, o) (x)

/**
 * Check if something has capability.
 *
 * @param x Capabilities to check in.
 * @param c Capability to check for.
 * @return \ref true if \p x has \p c, \ref false otherwise.
 */
#define has_cap(x, c) (x & c)

#endif /* KMI_CAPS_H */
