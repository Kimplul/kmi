/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_UTILS_H
#define KMI_UTILS_H

/**
 * @file utils.h
 * Misc utils and helpers.
 */

/**
 * Get absolute value of expression.
 *
 * @param a Value to take the absolute value of.
 * @return \c a if \c a >= 0, \c -a otherwise.
 * \sideeffects
 */
#define ABS(a) (a < 0 ? -a : a)

/**
 * Get the larger of two values.
 *
 * @param a First value.
 * @param b Second value.
 * @return \c a if \code a >= b \endcode, \c b otherwise.
 * \sideeffects
 */
#define MAX(a, b) ((a) >= (b) ? (a) : (b))

/**
 * Get the largest of three values.
 *
 * @param a First value.
 * @param b Second value.
 * @param c Third value.
 * @return Largest of the three values.
 * \sideeffects
 */
#define MAX3(a, b, c) (MAX(a, b) >= MAX(b, c) ? MAX(a, b) : MAX(b, c))

/**
 * Get the largest of four values.
 *
 * @param a First value.
 * @param b Second value.
 * @param c Third value.
 * @param d Fourth value.
 * @return Largest of the four values.
 * \sideeffects
 */
#define MAX4(a, b, c, d) \
	(MAX3(a, b, c) >= MAX3(b, c, d) ? MAX3(a, b, c) : MAX3(b, c, d))
/* etc... */

/**
 * Get the smaller of two values.
 *
 * @param a First value.
 * @param b Second value.
 * @return The larger of the two values.
 * \sideeffects
 */
#define MIN(a, b) ((a) <= (b) ? (a) : (b))

/**
 * Get the smallest of three values.
 *
 * @param a First value.
 * @param b Second value.
 * @param c Third value.
 * @return The largest of the three values.
 * \sideeffects
 */
#define MIN3(a, b, c) (MIN(a, b) <= MIN(b, c) ? MIN(a, b) : MIN(b, c))

/**
 * Get the largest of four values.
 *
 * @param a First value.
 * @param b Second value.
 * @param c Third value.
 * @param d Fourth value.
 * @return The largest of the three values.
 * \sideeffects
 */
#define MIN4(a, b, c, d) \
	(MIN3(a, b, c) <= MIN3(b, c, d) ? MIN3(a, b, c) : MIN3(b, c, d))
/* etc... */

/**
 * Helper for \ref GLUE().
 * Makes sure any macros are expanded all the way.
 *
 * @param x Head part of the string.
 * @param y Tail part of the string.
 * \see GLUE().
 */
#define GLUE2(x, y) x##y

/**
 * Glue two strings together.
 *
 * @param x Head part of the string.
 * @param y Tail part of the string.
 */
#define GLUE(x, y) GLUE2(x, y)

/**
 * Helper for \ref QUOTE().
 * Makes sure any macros are expanded all the way.
 *
 * @param x String to be quoted.
 * \see QUOTE().
 */
#define QUOTE2(x) #x

/**
 * Quote string.
 *
 * @param x String to be quoted.
 */
#define QUOTE(x) QUOTE2(x)

/**
 * Silence warnings about unused parameters.
 *
 * @param x Parameter that is unused.
 */
#define UNUSED(x) ((void)(x))

/**
 * Semantic wrapper around \ref UNUSED().
 * If something might be unused, depending on the configuration, it's more
 * accurate to say that it might be unused rather than stating that it is
 * unused.
 *
 * @param x Symbol that might be unused.
 */
#define MAYBE_UNUSED(x) UNUSED(x)

#include <kmi/builtin.h>

/**
 * Get offset of member inside structure.
 *
 * @param type Structure to look in.
 * @param member Member whose offset to get.
 * @return Offset of \c member in \c type.
 */
#if __has_builtin(__builtin_offsetof)
#define offsetof(type, member) __builtin_offsetof(type, member)
#else
#define offsetof(type, member) ((uintptr_t) &((type *)0)->member)
#endif

/**
 * Signal to the compiler that some expression is likely to be true.
 * Might aid in optimisation and branch prediction manipulation.
 *
 * @param x Expression that is likely to be true.
 * @return Value of \c x.
 *
 * Example:
 * \code if (likely(x)) { ... } \endcode
 *
 * \see unlikely()
 */
#if __has_builtin(__builtin_expect)
#define likely(x) __builtin_expect(!!(x), 1)
#else
#define likely(x) (x)
#endif

/**
 * Signal to the compiler that some expression is unlikely to be true.
 *
 * @param x Expression that is unlikely to be true.
 * @return Value of \c x.
 *
 * \see likely().
 */
#if __has_builtin(__builtin_expect)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define unlikely(x) (x)
#endif

/**
 * Signal to the compiler that some region is unreachable.
 * Mainly used for debugging with instrumentation, though it could provide some
 * micro-optimisations.
 */
#if __has_builtin(__builtin_unreachable)
#define unreachable() __builtin_unreachable()
#else
#define unreachable()
#endif

/**
 * Get container of some member.
 *
 * @param ptr Pointer to member in some structure.
 * @param type Structure member is part of.
 * @param member Member pointer is pointing to.
 * @return Pointer to structure itself instead of member.
 */
#define container_of(ptr, type, member) \
	((type *)((char *)(ptr)-offsetof(type, member)))

/**
 * Get array size in number of elements.
 *
 * @param x Array whose size should be calculated.
 * @return Number of elements in array.
 */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * Check if value is aligned to some other value.
 *
 * @param x Value whose alignment to check.
 * @param a Value to check alignment to.
 * @return \ref true if \c x is aligned, \ref false otherwise.
 */
#define ALIGNED(x, a) ((x) % a == 0)

/**
 * Add value to pointer in bytes instead of elements.
 *
 * @param x Pointer to add value to.
 * @param y Value to add to pointer.
 * @return \c x with \c y added to it.
 */
#define ptradd(x, y) (((vm_t)(x)) + ((vm_t)(y)))

#include <kmi/types.h>

/* clang-format doesn't like _Generic, but I guess that's fine.
 * Uncrustify just ignores it, as far as I can tell. */
/**
 * Align value upwards.
 * Type is deduced from \c x.
 *
 * @param x Value to align up.
 * @param y Value to align to.
 * @return \c x aligned to \c y.
 */
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

/**
 * Helper macro for defining type specific aligning.
 *
 * @param name Name of type in function name.
 * @param type Actual type.
 */
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

/**
 * Align signed char up.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned up to nearest multiple of \c a.
 */
DEFINE_ALIGN_UP(c, signed char);

/**
 * Align signed short up.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned up to nearest multiple of \c a.
 */
DEFINE_ALIGN_UP(s, signed short);

/**
 * Align signed int up.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned up to nearest multiple of \c a.
 */
DEFINE_ALIGN_UP(i, signed int);

/**
 * Align signed long up.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned up to nearest multiple of \c a.
 */
DEFINE_ALIGN_UP(l, signed long);

/**
 * Align signed long long up.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned up to nearest multiple of \c a.
 */
DEFINE_ALIGN_UP(ll, signed long long);

/**
 * Align unsigned char up.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned up to nearest multiple of \c a.
 */
DEFINE_ALIGN_UP(uc, unsigned char);

/**
 * Align unsigned short up.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned up to nearest multiple of \c a.
 */
DEFINE_ALIGN_UP(us, unsigned short);

/**
 * Align unsigned int up.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned up to nearest multiple of \c a.
 */
DEFINE_ALIGN_UP(ui, unsigned int);

/**
 * Align unsigned long up.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned up to nearest multiple of \c a.
 */
DEFINE_ALIGN_UP(ul, unsigned long);

/**
 * Align unsigned long long up.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned up to nearest multiple of \c a.
 */
DEFINE_ALIGN_UP(ull, unsigned long long);

/**
 * Align value downwards.
 * Type is deduced from \c x.
 *
 * @param x Value to align.
 * @param y Value to align to.
 * @return \c x aligned to down \c y.
 */
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

/**
 * Helper macro for defining type specific aligning.
 *
 * @param name Name of type in function name.
 * @param type Actual type.
 */
#define DEFINE_ALIGN_DOWN(name, type)                          \
	static inline type align_down_##name(type val, type a) \
	{                                                      \
		if (!a) {                                      \
			return val;                            \
		}                                              \
                                                               \
		return val - (val % a);                        \
	}

/**
 * Align signed char down.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned down to nearest multiple of \c a.
 */
DEFINE_ALIGN_DOWN(c, signed char);

/**
 * Align signed short down.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned down to nearest multiple of \c a.
 */
DEFINE_ALIGN_DOWN(s, signed short);

/**
 * Align signed int down.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned down to nearest multiple of \c a.
 */
DEFINE_ALIGN_DOWN(i, signed int);

/**
 * Align signed long down.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned down to nearest multiple of \c a.
 */
DEFINE_ALIGN_DOWN(l, signed long);

/**
 * Align signed long long down.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned down to nearest multiple of \c a.
 */
DEFINE_ALIGN_DOWN(ll, signed long long);

/**
 * Align unsigned char down.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned down to nearest multiple of \c a.
 */
DEFINE_ALIGN_DOWN(uc, unsigned char);

/**
 * Align unsigned short down.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned down to nearest multiple of \c a.
 */
DEFINE_ALIGN_DOWN(us, unsigned short);

/**
 * Align unsigned int down.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned down to nearest multiple of \c a.
 */
DEFINE_ALIGN_DOWN(ui, unsigned int);

/**
 * Align unsigned long down.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned down to nearest multiple of \c a.
 */
DEFINE_ALIGN_DOWN(ul, unsigned long);

/**
 * Align unsigned long long down.
 *
 * @param val Value to align.
 * @param a Value to align to.
 * @return \c val aligned down to nearest multiple of \c a.
 */
DEFINE_ALIGN_DOWN(ull, unsigned long long);

/**
 * Check if value is aligned.
 *
 * @param x Value to check.
 * @param y Alignment to check to.
 * @return \ref true if \c x is aligned to \c y, \ref false otherwise.
 */
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

/**
 * Helper macro for defining type specific alignment checks.
 *
 * @param name Name of type in function name.
 * @param type Actual type.
 */
#define DEFINE_ALIGNED(name, type)                             \
	static inline bool is_aligned_##name(type val, type a) \
	{                                                      \
		if (!a) {                                      \
			return true;                           \
		}                                              \
                                                               \
		return val % a == 0;                           \
	}

/**
 * Check if signed char is aligned.
 *
 * @param val Value to check.
 * @param a Alignment to check to.
 * @return \ref true if \c val is aligned to \c a, \ref false otherwise.
 */
DEFINE_ALIGNED(c, signed char);

/**
 * Check if signed short is aligned.
 *
 * @param val Value to check.
 * @param a Alignment to check to.
 * @return \ref true if \c val is aligned to \c a, \ref false otherwise.
 */
DEFINE_ALIGNED(s, signed short);

/**
 * Check if signed int is aligned.
 *
 * @param val Value to check.
 * @param a Alignment to check to.
 * @return \ref true if \c val is aligned to \c a, \ref false otherwise.
 */
DEFINE_ALIGNED(i, signed int);

/**
 * Check if signed long is aligned.
 *
 * @param val Value to check.
 * @param a Alignment to check to.
 * @return \ref true if \c val is aligned to \c a, \ref false otherwise.
 */
DEFINE_ALIGNED(l, signed long);

/**
 * Check if signed long long is aligned.
 *
 * @param val Value to check.
 * @param a Alignment to check to.
 * @return \ref true if \c val is aligned to \c a, \ref false otherwise.
 */
DEFINE_ALIGNED(ll, signed long long);

/**
 * Check if unsigned char is aligned.
 *
 * @param val Value to check.
 * @param a Alignment to check to.
 * @return \ref true if \c val is aligned to \c a, \ref false otherwise.
 */
DEFINE_ALIGNED(uc, unsigned char);

/**
 * Check if unsigned short is aligned.
 *
 * @param val Value to check.
 * @param a Alignment to check to.
 * @return \ref true if \c val is aligned to \c a, \ref false otherwise.
 */
DEFINE_ALIGNED(us, unsigned short);

/**
 * Check if unsigned int is aligned.
 *
 * @param val Value to check.
 * @param a Alignment to check to.
 * @return \ref true if \c val is aligned to \c a, \ref false otherwise.
 */
DEFINE_ALIGNED(ui, unsigned int);

/**
 * Check if unsigned long is aligned.
 *
 * @param val Value to check.
 * @param a Alignment to check to.
 * @return \ref true if \c val is aligned to \c a, \ref false otherwise.
 */
DEFINE_ALIGNED(ul, unsigned long);

/**
 * Check if unsigned long long is aligned.
 *
 * @param val Value to check.
 * @param a Alignment to check to.
 * @return \ref true if \c val is aligned to \c a, \ref false otherwise.
 */
DEFINE_ALIGNED(ull, unsigned long long);

/**
 * Get numeric value of ASCII character.
 *
 * @param c Character representing a number.
 * @return Corresponding number. -1 if parsing failed.
 */
static inline int asciinum(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else
		return -1;
}

/**
 * Convert string representing number to actual number.
 *
 * @param c String representing number.
 * @param len Length of \c str.
 * @param base Base of number.
 * @return Corresponding number.
 */
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

#endif /* KMI_UTILS_H */
