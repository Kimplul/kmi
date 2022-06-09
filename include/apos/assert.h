#ifndef APOS_ASSERT_H
#define APOS_ASSERT_H

/**
 * @file assert.h
 * Assertions. Note that contrary to how assertios usually function, apos has
 * three different levels of assertions: Catastrophic, hard and soft.
 *
 * Soft assertions merely warn about something that might cause issues, but let
 * the execution continue normally.
 *
 * Hard assertions warn about the assertion not holding and returns from the
 * function.
 *
 * Catastrophic assertions warn about the assertion and crash the kernel.
 */

#include <apos/debug.h>
#include <apos/utils.h>

/** \todo should this exit or do something explosive like that? */
#if !defined(DNDEBUG)

/**
 * The kernel is in an irrepairable state, just give up.
 *
 * @param x Condition to check for.
 */
#define catastrophic_assert(x)                                                  \
	do {                                                                    \
		if (unlikely(!(x))) {                                           \
			error("catastrophic assertion failed: " QUOTE(x) "\n"); \
			while (1) {                                             \
			}                                                       \
		}                                                               \
	} while (0);

/**
 * The function cannot continue without this assertion, but doesn't necessarily
 * mean that the kernel is borked.
 *
 * @warning Implicit return.
 *
 * @param x Condition to check for.
 * @param r Return value on failed check.
 */
#define hard_assert(x, r)                                              \
	{                                                              \
		if (unlikely(!(x))) {                                  \
			warn("hard assertion failed: " QUOTE(x) "\n"); \
			return r;                                      \
		}                                                      \
	}

/**
 * Unexpected case, but not likely to cause problems, likely a bug.
 *
 * @param x Condition to check for.
 */
#define soft_assert(x)                                                  \
	do {                                                            \
		if (unlikely(!(x))) {                                   \
			info("soft assertion failed: "  QUOTE(x) "\n"); \
		}                                                       \
	} while (0);
#else
#define catastrophic_assert(x)
#define hard_assert(x, r)
#define soft_assert(x)
#endif

/**
 * Use when return value doesn't exist.
 *
 * Example:
 * @code{.c}
 * void func() { hard_assert(x, RETURN_VOID); }
 * @endcode
 */
#define RETURN_VOID

#endif /* APOS_ASSERT_H */
