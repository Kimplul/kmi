#ifndef APOS_CAPS_H
#define APOS_CAPS_H

/**
 * @file caps.h
 * Capabilities of threads.
 * \todo The list of capabilities should maybe be placed in some other file so
 * as to easier extract it into userspace programs.
 */

#include <apos/bits.h>

/** Helper typedef for capabilities. */
typedef unsigned char capflags_t;

enum {
	/** Thread is allowed to set capabilities of other threads. */
	CAP_CAPS = (1 << 0),

	/** Thread is allowed to modify process statuses, exec/fork/etc. */
	CAP_PROC = (1 << 1),

	/** Thread is allowed to force interrupt to callback in other thread. */
	CAP_CALL = (1 << 2),
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

#endif /* APOS_CAPS_H */
