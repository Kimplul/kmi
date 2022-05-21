#ifndef APOS_ASSERT_H
#define APOS_ASSERT_H

#include <apos/debug.h>
#include <apos/utils.h>

/* TODO: should this exit or do something explosive like that? */
#if !defined(DNDEBUG)
#define catastrophic_assert(x)                                                 \
	do {                                                                   \
		if (x) {                                                       \
			err("catastrophic assertion failed: %s\n", QUOTE(x));  \
			while (1)                                              \
				;                                              \
		}                                                              \
	} while (0);

#define hard_assert(x, r)                                                      \
	do {                                                                   \
		if (x) {                                                       \
			warn("hard assertion failed: %s\n", QUOTE(x));         \
			return r;                                              \
		}                                                              \
	} while (0);

#define soft_assert(x)                                                         \
	do {                                                                   \
		if (x) {                                                       \
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
