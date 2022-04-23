#ifndef APOS_UTILS_H
#define APOS_UTILS_H

#define ABS(a)        (a < 0 ? -a : a)

#define MAX(a, b)     ((a) >= (b) ? (a) : (b))
#define MAX3(a, b, c) (MAX(a, b) >= MAX(b, c) ? MAX(a, b) : MAX(b, c))
#define MAX4(a, b, c, d)                                                       \
	(MAX3(a, b, c) >= MAX3(b, c, d) ? MAX3(a, b, c) : MAX3(b, c, d))
/* etc... */

#define MIN(a, b)     ((a) <= (b) ? (a) : (b))
#define MIN3(a, b, c) (MIN(a, b) <= MIN(b, c) ? MIN(a, b) : MIN(b, c))
#define MIN4(a, b, c, d)                                                       \
	(MIN3(a, b, c) <= MIN3(b, c, d) ? MIN3(a, b, c) : MIN3(b, c, d))
/* etc... */

#define GLUE2(x, y) x##y
#define GLUE(x, y)  GLUE2(x, y)

#define QUOTE2(x)   #x
#define QUOTE(x)    QUOTE2(x)

#define UNUSED(x)   ((void)(x))

#include <apos/builtin.h>

#if __has_builtin(__builtin_offsetof)
#define offsetof(type, member) __builtin_offsetof(type, member)
#else
#define offsetof(type, member) ((uintptr_t) & ((type *)0)->member)
#endif

#if __has_builtin(__builtin_expect)
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define container_of(ptr, type, member)                                        \
	((type *)((char *)(ptr)-offsetof(type, member)))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED(x, a) ((x) % a == 0)

#define ptradd(x, y)  (((vm_t)(x)) + ((vm_t)(y)))

#include <apos/types.h>

#define align_up(x, y)                                                         \
	_Generic((x), int8_t                                                   \
	         : align_up_int8_t, int16_t                                    \
	         : align_up_int16_t, int32_t                                   \
	         : align_up_int32_t, int64_t                                   \
	         : align_up_int64_t,                                           \
                                                                               \
	           uint8_t                                                     \
	         : align_up_uint8_t, uint16_t                                  \
	         : align_up_uint16_t, uint32_t                                 \
	         : align_up_uint32_t, uint64_t                                 \
	         : align_up_uint64_t)((x), (y))

#define DEFINE_ALIGN_UP(type)                                                  \
	static inline type align_up_##type(type val, type a)                   \
	{                                                                      \
		if (!a)                                                        \
			return val;                                            \
                                                                               \
		type rem = val % a;                                            \
                                                                               \
		if (rem == 0)                                                  \
			return val;                                            \
                                                                               \
		return val + a - rem;                                          \
	}

DEFINE_ALIGN_UP(int8_t);
DEFINE_ALIGN_UP(int16_t);
DEFINE_ALIGN_UP(int32_t);
DEFINE_ALIGN_UP(int64_t);

DEFINE_ALIGN_UP(uint8_t);
DEFINE_ALIGN_UP(uint16_t);
DEFINE_ALIGN_UP(uint32_t);
DEFINE_ALIGN_UP(uint64_t);

#define align_down(x, y)                                                       \
	_Generic((x), int8_t                                                   \
	         : align_down_int8_t, int16_t                                  \
	         : align_down_int16_t, int32_t                                 \
	         : align_down_int32_t, int64_t                                 \
	         : align_down_int64_t,                                         \
                                                                               \
	           uint8_t                                                     \
	         : align_down_uint8_t, uint16_t                                \
	         : align_down_uint16_t, uint32_t                               \
	         : align_down_uint32_t, uint64_t                               \
	         : align_down_uint64_t)((x), (y))

#define DEFINE_ALIGN_DOWN(type)                                                \
	static inline type align_down_##type(type val, type a)                 \
	{                                                                      \
		if (!a)                                                        \
			return val;                                            \
                                                                               \
		return val - (val % a);                                        \
	}

DEFINE_ALIGN_DOWN(int8_t);
DEFINE_ALIGN_DOWN(int16_t);
DEFINE_ALIGN_DOWN(int32_t);
DEFINE_ALIGN_DOWN(int64_t);

DEFINE_ALIGN_DOWN(uint8_t);
DEFINE_ALIGN_DOWN(uint16_t);
DEFINE_ALIGN_DOWN(uint32_t);
DEFINE_ALIGN_DOWN(uint64_t);

#define is_aligned(x, y)                                                       \
	_Generic((x), int8_t                                                   \
	         : is_aligned_int8_t, int16_t                                  \
	         : is_aligned_int16_t, int32_t                                 \
	         : is_aligned_int32_t, int64_t                                 \
	         : is_aligned_int64_t,                                         \
                                                                               \
	           uint8_t                                                     \
	         : is_aligned_uint8_t, uint16_t                                \
	         : is_aligned_uint16_t, uint32_t                               \
	         : is_aligned_uint32_t, uint64_t                               \
	         : is_aligned_uint64_t)((x), (y))

#define DEFINE_ALIGNED(type)                                                   \
	static inline bool is_aligned_##type(type val, type a)                 \
	{                                                                      \
		if (!a)                                                        \
			return true;                                           \
                                                                               \
		return val % a == 0;                                           \
	}

DEFINE_ALIGNED(int8_t);
DEFINE_ALIGNED(int16_t);
DEFINE_ALIGNED(int32_t);
DEFINE_ALIGNED(int64_t);

DEFINE_ALIGNED(uint8_t);
DEFINE_ALIGNED(uint16_t);
DEFINE_ALIGNED(uint32_t);
DEFINE_ALIGNED(uint64_t);

static inline int asciinum(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else
		return 0;
}

static inline uintmax_t convnum(const char *c, size_t len, size_t base)
{
	size_t multiplier = 1;
	size_t sum = 0;
	for (size_t i = 0; i < len; ++i) {
		sum += asciinum(c[len - 1 - i]) * multiplier;
		multiplier *= base;
	}

	return sum;
}

#endif /* APOS_UTILS_H */
