#ifndef APOS_TYPES_H
#define APOS_TYPES_H

#include <apos/builtin.h>

typedef _Bool	bool;
#define true	1
#define false	0

typedef __PTRDIFF_TYPE__	ptrdiff_t;
typedef __WCHAR_TYPE__		wchar_t;
typedef __WINT_TYPE__		wint_t;
typedef __INTMAX_TYPE__		intmax_t;
typedef __UINTMAX_TYPE__	uintmax_t;
typedef __INT8_TYPE__		int8_t;
typedef __INT16_TYPE__		int16_t;
typedef __INT32_TYPE__		int32_t;
typedef __INT64_TYPE__		int64_t;
typedef __UINT8_TYPE__		uint8_t;
typedef __UINT16_TYPE__		uint16_t;
typedef __UINT32_TYPE__		uint32_t;
typedef __UINT64_TYPE__		uint64_t;
typedef __INT_LEAST8_TYPE__	int_least8_t;
typedef __INT_LEAST16_TYPE__	int_least16_t;
typedef __INT_LEAST32_TYPE__	int_least32_t;
typedef __INT_LEAST64_TYPE__	int_least64_t;
typedef __UINT_LEAST8_TYPE__	uint_least8_t;
typedef __UINT_LEAST16_TYPE__	uint_least16_t;
typedef __UINT_LEAST32_TYPE__	uint_least32_t;
typedef __UINT_LEAST64_TYPE__	uint_least64_t;
typedef __INT_FAST8_TYPE__	int_fast8_t;
typedef __INT_FAST16_TYPE__	int_fast16_t;
typedef __INT_FAST32_TYPE__	int_fast32_t;
typedef __INT_FAST64_TYPE__	int_fast64_t;
typedef __UINT_FAST8_TYPE__	uint_fast8_t;
typedef __UINT_FAST16_TYPE__	uint_fast16_t;
typedef __UINT_FAST32_TYPE__	uint_fast32_t;
typedef __UINT_FAST64_TYPE__	uint_fast64_t;
typedef __INTPTR_TYPE__		intptr_t;
typedef __UINTPTR_TYPE__	uintptr_t;

/* sort of unconventional, but should be fine */
typedef uintmax_t size_t;
typedef intmax_t ssize_t;

#define INT8_C		__INT8_C
#define INT16_C		__INT16_C
#define INT32_C		__INT32_C
#define INT64_C		__INT64_C
#define UINT8_C		__UINT8_C
#define UINT16_C	__UINT16_C
#define UINT32_C	__UINT32_C
#define UINT64_C	__UINT64_C
#define INTMAX_C	__INTMAX_C
#define UINTMAX_C	__UINTMAX_C

#define CHAR_BIT	__CHAR_BIT__

#define SCHAR_MAX	__SCHAR_MAX__
#define SCHAR_MIN	(-__SCHAR_MAX - 1)
#define UCHAR_MAX	(2 * __SCHAR_MAX__ - 1)

#ifdef __CHAR_UNSIGNED__
#define CHAR_MIN	0
#define CHAR_MAX	UCHAR_MAX
#else
#define CHAR_MIN	SCHAR_MIN
#define CHAR_MAX	SCHAR_MAX
#endif

/* probably not necessary? */
#define MB_LEN_MAX	16

#define SHRT_MAX	__SHRT_MAX__
#define SHRT_MIN	(-__SHRT_MAX - 1)
#define USHRT_MAX	(2 * __SHRT_MAX + 1)

#define INT_MAX		__INT_MAX__
#define INT_MIN		(-__INT_MAX__ - 1)
#define UINT_MAX	(2 * __INT_MAX__ + 1)

#define LONG_MAX	__LONG__MAX__
#define LONG_MIN	(-__LONG_MAX__ - 1)
#define ULONG_MAX	(2 * __INT_MAX__ + 1)

#define LLONG_MAX	__LONG_LONG_MAX__
#define LLONG_MIN	(-__LONG_LONG_MAX__ - 1)
#define ULLONG_MAX	(2 * __LONG_LONG_MAX__ + 1)

#define INT8_MAX	__INT8_MAX__
#define INT8_MIN	(-__INT8_MAX__ - 1)
#define UINT8_MAX	__UINT8_MAX__

#define INT16_MAX	__INT16_MAX__
#define INT16_MIN	(-__INT16_MAX__ - 1)
#define UINT16_MAX	__UINT16_MAX__

#define INT32_MAX	__INT32_MAX__
#define INT32_MIN	(-__INT32_MAX__ - 1)
#define UINT32_MAX	__UINT32_MAX__

#define INT64_MAX	__INT64_MAX__
#define INT64_MIN	(-__INT64_MAX__ - 1)
#define UINT64_MAX	__UINT64_MAX__

#define INT_LEAST8_MAX	__INT_LEAST8_MAX__
#define INT_LEAST8_MIN	(-__INT_LEAST8_MAX__ - 1)
#define UINT_LEAST8_MAX __UINT_LEAST8_MAX__

#define INT_LEAST16_MAX	__INT_LEAST16_MAX__
#define INT_LEAST16_MIN	(-__INT_LEAST16_MAX__ - 1)
#define UINT_LEAST16_MAX	__UINT_LEAST16_MAX__

#define INT_LEAST32_MAX	__INT_LEAST32_MAX__
#define INT_LEAST32_MIN	(-__INT_LEAST32_MAX__ - 1)
#define UINT_LEAST32_MAX	__UINT_LEAST32_MAX__

#define INT_LEAST64_MAX	__INT_LEAST64_MAX__
#define INT_LEAST64_MIN	(-__INT_LEAST64_MAX__ - 1)
#define UINT_LEAST64_MAX	__UINT_LEAST64_MAX__

#define INT_FAST8_MAX	__INT_FAST8_MAX__
#define INT_FAST8_MIN	(-__INT_FAST8_MAX__ - 1)
#define UINT_FAST8_MAX __UINT_FAST8_MAX__

#define INT_FAST16_MAX	__INT_FAST16_MAX__
#define INT_FAST16_MIN	(-__INT_FAST16_MAX__ - 1)
#define UINT_FAST16_MAX	__UINT_FAST16_MAX__

#define INT_FAST32_MAX	__INT_FAST32_MAX__
#define INT_FAST32_MIN	(-__INT_FAST32_MAX__ - 1)
#define UINT_FAST32_MAX __UINT_FAST32_MAX__

#define INT_FAST64_MAX	__INT_FAST64_MAX__
#define INT_FAST64_MIN	(-__INT_FAST64_MAX__ - 1)
#define UINT_FAST64_MAX	__UINT_FAST64_MAX__

#define INTPTR_MAX	__INTPTR_MAX__
#define INTPTR_MIN	(-__INTPTR_MAX__ - 1)
#define UINTPTR_MAX	__UINTPTR_MAX__

#define INTMAX_MAX	__INTMAX_MAX__
#define INTMAX_MIN	(-__INTMAX_MAX__ - 1)
#define UINTMAX_MAX	__UINTMAX_MAX__

#define INT8_WIDTH	8
#define INT16_WIDTH	16
#define INT32_WIDTH	32
#define INT64_WIDTH	64
#define UINT8_WIDTH	8
#define UINT16_WIDTH	16
#define UINT32_WIDTH	32
#define UINT64_WIDTH	64

#define INT_FAST8_WIDTH		__INT_FAST8_WIDTH__
#define INT_FAST16_WIDTH	__INT_FAST16_WIDTH__
#define INT_FAST32_WIDTH	__INT_FAST32_WIDTH__
#define INT_FAST64_WIDTH	__INT_FAST64_WIDTH__
#define UINT_FAST8_WIDTH	__INT_FAST8_WIDTH__
#define UINT_FAST16_WIDTH	__INT_FAST16_WIDTH__
#define UINT_FAST32_WIDTH	__INT_FAST32_WIDTH__
#define UINT_FAST64_WIDTH	__INT_FAST64_WIDTH__

#define INT_LEAST8_WIDTH	__INT_LEAST8_WIDTH__
#define INT_LEAST16_WIDTH	__INT_LEAST16_WIDTH__
#define INT_LEAST32_WIDTH	__INT_LEAST32_WIDTH__
#define INT_LEAST64_WIDTH	__INT_LEAST64_WIDTH__
#define UINT_LEAST8_WIDTH	__INT_LEAST8_WIDTH__
#define UINT_LEAST16_WIDTH	__INT_LEAST16_WIDTH__
#define UINT_LEAST32_WIDTH	__INT_LEAST32_WIDTH__
#define UINT_LEAST64_WIDTH	__INT_LEAST64_WIDTH__

#define INTPTR_WIDTH		__INTPTR_WIDTH__
#define INTMAX_WIDTH		__INTMAX_WIDTH__
#define UINTPTR_WIDTH		__INTPTR_WIDTH__
#define UINTMAX_WIDTH		__INTMAX_WIDTH__

#define NULL 0

#endif /* APOS_TYPES_H */
