#ifndef APOS_UTILS_H
#define APOS_UTILS_H

/**
 * @file utils.h
 * Misc utils and helpers.
 */

#define ABS(a) (a < 0 ? -a : a)

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MAX3(a, b, c) (MAX(a, b) >= MAX(b, c) ? MAX(a, b) : MAX(b, c))
#define MAX4(a, b, c, d) \
	(MAX3(a, b, c) >= MAX3(b, c, d) ? MAX3(a, b, c) : MAX3(b, c, d))
/* etc... */

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MIN3(a, b, c) (MIN(a, b) <= MIN(b, c) ? MIN(a, b) : MIN(b, c))
#define MIN4(a, b, c, d) \
	(MIN3(a, b, c) <= MIN3(b, c, d) ? MIN3(a, b, c) : MIN3(b, c, d))
/* etc... */

#define GLUE2(x, y) x##y
#define GLUE(x, y) GLUE2(x, y)

#define QUOTE2(x) #x
#define QUOTE(x) QUOTE2(x)

#define UNUSED(x) ((void)(x))
#define MAYBE_UNUSED(x) UNUSED(x)

#include <apos/builtin.h>

#if __has_builtin(__builtin_offsetof)
#define offsetof(type, member) __builtin_offsetof(type, member)
#else
#define offsetof(type, member) ((uintptr_t) &((type *)0)->member)
#endif

#if __has_builtin(__builtin_expect)
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr)-offsetof(type, member)))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED(x, a) ((x) % a == 0)

#define ptradd(x, y) (((vm_t)(x)) + ((vm_t)(y)))

#include <apos/types.h>

/* clang-format doesn't like _Generic, but I guess that's fine. */
#define align_up(x, y)                             \
	_Generic((x), signed char                  \
	         : align_up_c, signed short        \
	         : align_up_s, signed int          \
	         : align_up_i, signed long         \
	         : align_up_l, signed long long    \
	         : align_up_ll,                    \
                                                   \
	         unsigned char                     \
	         : align_up_uc, unsigned short     \
	         : align_up_us, unsigned int       \
	         : align_up_ui, unsigned long      \
	         : align_up_ul, unsigned long long \
	         : align_up_ull)((x), (y))

#define DEFINE_ALIGN_UP(name, type)                          \
	static inline type align_up_##name(type val, type a) \
	{                                                    \
		if (!a) {                                    \
			return val;                          \
		}                                            \
                                                             \
		type rem = val % a;                          \
                                                             \
		if (rem == 0) {                              \
			return val;                          \
		}                                            \
                                                             \
		return val + a - rem;                        \
	}

DEFINE_ALIGN_UP(c, signed char);
DEFINE_ALIGN_UP(s, signed short);
DEFINE_ALIGN_UP(i, signed int);
DEFINE_ALIGN_UP(l, signed long);
DEFINE_ALIGN_UP(ll, signed long long);

DEFINE_ALIGN_UP(uc, unsigned char);
DEFINE_ALIGN_UP(us, unsigned short);
DEFINE_ALIGN_UP(ui, unsigned int);
DEFINE_ALIGN_UP(ul, unsigned long);
DEFINE_ALIGN_UP(ull, unsigned long long);

#define align_down(x, y)                             \
	_Generic((x), signed char                    \
	         : align_down_c, signed short        \
	         : align_down_s, signed int          \
	         : align_down_i, signed long         \
	         : align_down_l, signed long long    \
	         : align_down_ll,                    \
                                                     \
	         unsigned char                       \
	         : align_down_uc, unsigned short     \
	         : align_down_us, unsigned int       \
	         : align_down_ui, unsigned long      \
	         : align_down_ul, unsigned long long \
	         : align_down_ull)((x), (y))

#define DEFINE_ALIGN_DOWN(name, type)                          \
	static inline type align_down_##name(type val, type a) \
	{                                                      \
		if (!a) {                                      \
			return val;                            \
		}                                              \
                                                               \
		return val - (val % a);                        \
	}

DEFINE_ALIGN_DOWN(c, signed char);
DEFINE_ALIGN_DOWN(s, signed short);
DEFINE_ALIGN_DOWN(i, signed int);
DEFINE_ALIGN_DOWN(l, signed long);
DEFINE_ALIGN_DOWN(ll, signed long long);

DEFINE_ALIGN_DOWN(uc, unsigned char);
DEFINE_ALIGN_DOWN(us, unsigned short);
DEFINE_ALIGN_DOWN(ui, unsigned int);
DEFINE_ALIGN_DOWN(ul, unsigned long);
DEFINE_ALIGN_DOWN(ull, unsigned long long);

#define is_aligned(x, y)                             \
	_Generic((x), signed char                    \
	         : is_aligned_c, signed short        \
	         : is_aligned_s, signed int          \
	         : is_aligned_i, signed long         \
	         : is_aligned_l, signed long long    \
	         : is_aligned_ll,                    \
                                                     \
	         unsigned char                       \
	         : is_aligned_uc, unsigned short     \
	         : is_aligned_us, unsigned int       \
	         : is_aligned_ui, unsigned long      \
	         : is_aligned_ul, unsigned long long \
	         : is_aligned_ll)((x), (y))

#define DEFINE_ALIGNED(name, type)                             \
	static inline bool is_aligned_##name(type val, type a) \
	{                                                      \
		if (!a) {                                      \
			return true;                           \
		}                                              \
                                                               \
		return val % a == 0;                           \
	}

DEFINE_ALIGNED(c, signed char);
DEFINE_ALIGNED(s, signed short);
DEFINE_ALIGNED(i, signed int);
DEFINE_ALIGNED(l, signed long);
DEFINE_ALIGNED(ll, signed long long);

DEFINE_ALIGNED(uc, unsigned char);
DEFINE_ALIGNED(us, unsigned short);
DEFINE_ALIGNED(ui, unsigned int);
DEFINE_ALIGNED(ul, unsigned long);
DEFINE_ALIGNED(ull, unsigned long long);

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
