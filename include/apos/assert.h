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

/* TODO: should this exit or do something explosive like that? */
#if !defined(DNDEBUG)
#define catastrophic_assert(x)                                                 \
	do {                                                                   \
		if (unlikely(!(x))) {                                          \
			error("catastrophic assertion failed: %s\n",           \
			      QUOTE(x));                                       \
			while (1) {                                            \
			}                                                      \
		}                                                              \
	} while (0);

#define hard_assert(x, r)                                                      \
	{                                                                      \
		if (unlikely(!(x))) {                                          \
			warn("hard assertion failed: %s\n", QUOTE(x));         \
			return r;                                              \
		}                                                              \
	}

#define soft_assert(x)                                                         \
	do {                                                                   \
		if (unlikely(!(x))) {                                          \
			info("soft assertion failed: %s\n", QUOTE(x));         \
		}                                                              \
	} while (0);
#else
#define catastrophic_assert(x)
#define hard_assert(x, r)
#define soft_assert(x)
#endif

/* use when return value doesn't exist, like hard_assert(x,
 * RETURN_VOID); */
#define RETURN_VOID

#endif /* APOS_ASSERT_H */
