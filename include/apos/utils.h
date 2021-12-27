#ifndef APOS_UTILS_H
#define APOS_UTILS_H

#define ABS(a) (a < 0 ? -a : a)

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MAX3(a, b, c) (MAX(a, b) >= MAX(b, c) ? MAX(a, b) : MAX(b, c))
#define MAX4(a, b, c, d) (MAX3(a, b, c) >= MAX3(b, c, d) ? MAX3(a, b, c) : MAX3(b, c, d))
/* etc... */

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MIN3(a, b, c) (MIN(a, b) <= MIN(b, c) ? MIN(a, b) : MIN(b, c))
#define MIN4(a, b, c, d) (MIN3(a, b, c) <= MIN3(b, c, d) ? MIN3(a, b, c) : MIN3(b, c, d))
/* etc... */

#define GLUE2(x, y) x##y
#define GLUE(x, y) GLUE2(x, y)

#define QUOTE2(x) #x
#define QUOTE(x) QUOTE2(x)

#define UNUSED(x) ((void)(x))

#include <apos/builtin.h>

#if __has_builtin(__builtin_offsetof)
#define offsetof(type, member) __builtin_offsetof(type, member)
#else
#define offsetof(type, member) ((size_t)&((type *)0)->member)
#endif

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED(x, a) ((x) % a == 0)

#define ptradd(x, y) (((vm_t)(x)) + ((vm_t)(y)))

#include <apos/types.h>

static inline size_t align_up(size_t val, size_t a)
{
	if(!a)
		return val;

	size_t rem = val % a;

	if (rem == 0)
		return val;

	return val + a - rem;
}

static inline size_t align_down(size_t val, size_t a)
{
	if(!a)
		return val;

	return val - (val % a);
}

static inline bool aligned(size_t val, size_t a)
{
	if (!a)
		return true;

	return val % a == 0;
}

static inline size_t asciinum(char c)
{
	if(c >= '0' && c <= '9')
		return c - '0';
	else if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else
		return 0;
}

static inline size_t convnum(const char *c, size_t len, size_t base)
{
	size_t multiplier = 1;
	size_t sum = 0;
	for(size_t i = 0; i < len; ++i){
		sum += asciinum(c[len - 1 - i]) * multiplier;
		multiplier *= base;
	}

	return sum;
}


#endif /* APOS_UTILS_H */
