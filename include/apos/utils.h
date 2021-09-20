#ifndef APOS_UTILS_H
#define APOS_UTILS_H

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MAX3(a, b, c) (MAX(a, b) >= MAX(b, c) ? MAX(a, b) : MAX(b, c))
#define MAX4(a, b, c, d) (MAX3(a, b, c) >= MAX3(b, c, d) ? MAX3(a, b, c) : MAX3(b, c, d))
/* etc... */

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MIN3(a, b, c) (MIN(a, b) <= MIN(b, c) ? MIN(a, b) : MIN(b, c))
#define MIN4(a, b, c, d) (MIN3(a, b, c) <= MIN3(b, c, d) ? MIN3(a, b, c) : MIN3(b, c, d))
/* etc... */

#include <apos/types.h>

static inline size_t align_up(size_t val, size_t a)
{
	size_t rem = val % a;

	if (rem == 0)
		return val;

	return val + a - rem;
}

static inline size_t align_down(size_t val, size_t a)
{
	return val - (val % a);
}

#endif /* APOS_UTILS_H */
