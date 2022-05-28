#ifndef APOS_TYPES_H
#define APOS_TYPES_H

/**
 * @file types.h
 * Shorthands for types, similar to stdint.h.
 *
 * Implementation detail: Note that according to the C spec, intX_t are optional.
 * Will have to keep an eye out for architectures where they aren't implemented,
 * but for now this is likely good enough.
 */

/** Bool. */
typedef _Bool bool;
/** True. */
#define true 1
/** False. */
#define false 0

/** Type capable of holding the difference between two pointers. */
typedef __PTRDIFF_TYPE__ ptrdiff_t;

/** Type capable of holding a wide char. Unused by the kernel. */
typedef __WCHAR_TYPE__ wchar_t;

/** Type capable of holding a \ref wchar_t and WEOF. Unused by the kernel. */
typedef __WINT_TYPE__ wint_t;

/** Maximum width signed integer type. */
typedef __INTMAX_TYPE__ intmax_t;

/** Maximum width unsigned integer type. */
typedef __UINTMAX_TYPE__ uintmax_t;

/** 8bit signed integer type. */
typedef __INT8_TYPE__ int8_t;

/** 16bit signed integer type. */
typedef __INT16_TYPE__ int16_t;

/** 32bit signed integer type. */
typedef __INT32_TYPE__ int32_t;

/** 64bit signed integer type. */
typedef __INT64_TYPE__ int64_t;

/** 8bit unsigned integer type. */
typedef __UINT8_TYPE__ uint8_t;

/** 16bit unsigned integer type. */
typedef __UINT16_TYPE__ uint16_t;

/** 32bit unsigned integer type. */
typedef __UINT32_TYPE__ uint32_t;

/** 64bit unsigned integer type. */
typedef __UINT64_TYPE__ uint64_t;

/** Smallest signed integer type at least 8 bits wide. */
typedef __INT_LEAST8_TYPE__ int_least8_t;

/** Smallest signed integer type at least 16 bits wide. */
typedef __INT_LEAST16_TYPE__ int_least16_t;

/** Smallest signed integer type at least 32 bits wide. */
typedef __INT_LEAST32_TYPE__ int_least32_t;

/** Smallest signed integer type at least 64 bits wide. */
typedef __INT_LEAST64_TYPE__ int_least64_t;

/** Smallest unsigned integer type at least 8 bits wide. */
typedef __UINT_LEAST8_TYPE__ uint_least8_t;

/** Smallest unsigned integer type at least 16 bits wide. */
typedef __UINT_LEAST16_TYPE__ uint_least16_t;

/** Smallest unsigned integer type at least 32 bits wide. */
typedef __UINT_LEAST32_TYPE__ uint_least32_t;

/** Smallest unsigned integer type at least 64 bits wide. */
typedef __UINT_LEAST64_TYPE__ uint_least64_t;

/** Fastest signed integer type at least 8 bits wide. */
typedef __INT_FAST8_TYPE__ int_fast8_t;

/** Fastest signed integer type at least 16 bits wide. */
typedef __INT_FAST16_TYPE__ int_fast16_t;

/** Fastest signed integer type at least 32 bits wide. */
typedef __INT_FAST32_TYPE__ int_fast32_t;

/** Fastest signed integer type at least 64 bits wide. */
typedef __INT_FAST64_TYPE__ int_fast64_t;

/** Fastest unsigned integer type at least 8 bits wide. */
typedef __UINT_FAST8_TYPE__ uint_fast8_t;

/** Fastest unsigned integer type at least 16 bits wide. */
typedef __UINT_FAST16_TYPE__ uint_fast16_t;

/** Fastest unsigned integer type at least 32 bits wide. */
typedef __UINT_FAST32_TYPE__ uint_fast32_t;

/** Fastest unsigned integer type at least 64 bits wide. */
typedef __UINT_FAST64_TYPE__ uint_fast64_t;

/** Signed integer type capable of holding a pointer. */
typedef __INTPTR_TYPE__ intptr_t;

/** Unsigned integer type capable of holding a pointer. */
typedef __UINTPTR_TYPE__ uintptr_t;

#if __SIZE_WIDTH__ == 64
/** Largest possible unsigned index an array could use. */
typedef uint64_t size_t;

/** Largest possible signed index an array could use. */
typedef int64_t ssize_t;
#else
/** Largest possible unsigned index an array could use. */
typedef uint32_t size_t;

/** Largest possible signed index an array could use. */
typedef int32_t ssize_t;
#endif

/** Expands to integer constant of type \ref int8_t. */
#define INT8_C    __INT8_C

/** Expands to integer constant of type \ref int16_t. */
#define INT16_C   __INT16_C

/** Expands to integer constant of type \ref int32_t. */
#define INT32_C   __INT32_C
/** Expands to integer constant of type \ref int64_t. */
#define INT64_C   __INT64_C

/** Expands to integer constant of type \ref uint8_t. */
#define UINT8_C   __UINT8_C

/** Expands to integer constant of type \ref uint16_t. */
#define UINT16_C  __UINT16_C

/** Expands to integer constant of type \ref uint32_t. */
#define UINT32_C  __UINT32_C

/** Expands to integer constant of type \ref uint64_t. */
#define UINT64_C  __UINT64_C

/** Expands to integer constant of type \ref intmax_t. */
#define INTMAX_C  __INTMAX_C

/** Expands to integer constant of type \ref uintmax_t. */
#define UINTMAX_C __UINTMAX_C

/** Number of bits in a byte. */
#define CHAR_BIT  __CHAR_BIT__

/** Largest value a signed char can have. */
#define SCHAR_MAX __SCHAR_MAX__

/** Smallest value a signed char can have. */
#define SCHAR_MIN (-__SCHAR_MAX - 1)

/** Largest value an unsigned char can have. */
#define UCHAR_MAX (2 * __SCHAR_MAX__ - 1)

#if defined(__CHAR_UNSIGNED__)
/** Smallest value a char can have. */
#define CHAR_MIN 0

/** Largest value a char can have. */
#define CHAR_MAX UCHAR_MAX
#else
/** Smallest value a char can have. */
#define CHAR_MIN SCHAR_MIN

/** Largest value a char can have. */
#define CHAR_MAX SCHAR_MAX
#endif

/**
 * Maximum number of bytes in a multibyte character.
 * Arbitrary, as far as I can tell.
 */
#define MB_LEN_MAX         16

/** Largest value a signed short can have. */
#define SHRT_MAX           __SHRT_MAX__

/** Smallest value a signed short can have. */
#define SHRT_MIN           (-__SHRT_MAX - 1)

/** Largest value an unsigned short can have. */
#define USHRT_MAX          (2 * __SHRT_MAX + 1)

/** Largest value a signed int can have. */
#define INT_MAX            __INT_MAX__

/** Smallest value a signed int can have. */
#define INT_MIN            (-__INT_MAX__ - 1)

/** Largest value an unsigned int can have. */
#define UINT_MAX           (2 * __INT_MAX__ + 1)

/** Largest value a signed long can have. */
#define LONG_MAX           __LONG__MAX__

/** Smallest value a signed long can have. */
#define LONG_MIN           (-__LONG_MAX__ - 1)

/** Largest value an unsigned long can have. */
#define ULONG_MAX          (2 * __INT_MAX__ + 1)

/** Largest value a signed long long can have. */
#define LLONG_MAX          __LONG_LONG_MAX__

/** Smallest value a signed long long can have. */
#define LLONG_MIN          (-__LONG_LONG_MAX__ - 1)

/** Largest value an unsigned long long can have. */
#define ULLONG_MAX         (2 * __LONG_LONG_MAX__ + 1)

/** Largest value an \ref int8_t can have. */
#define INT8_MAX           __INT8_MAX__

/** Smallest value an \ref int8_t can have. */
#define INT8_MIN           (-__INT8_MAX__ - 1)

/** Largest value an \ref int8_t can have. */
#define UINT8_MAX          __UINT8_MAX__

/** Largest value an \ref int16_t can have. */
#define INT16_MAX          __INT16_MAX__

/** Smallest value an \ref int16_t can have. */
#define INT16_MIN          (-__INT16_MAX__ - 1)

/** Largest value an \ref uint16_t can have. */
#define UINT16_MAX         __UINT16_MAX__

/** Largest value an \ref int32_t can have. */
#define INT32_MAX          __INT32_MAX__

/** Smallest value an \ref int32_t can have. */
#define INT32_MIN          (-__INT32_MAX__ - 1)

/** Largest value a \ref uint16_t can have. */
#define UINT32_MAX         __UINT32_MAX__

/** Largest value an \ref int64_t can have. */
#define INT64_MAX          __INT64_MAX__

/** Smallest value an \ref int64_t can have. */
#define INT64_MIN          (-__INT64_MAX__ - 1)

/** Largest value an \ref uint64_t can have. */
#define UINT64_MAX         __UINT64_MAX__

/** Largest value an \ref int_least8_t can have. */
#define INT_LEAST8_MAX     __INT_LEAST8_MAX__

/** Smallest value an \ref int_least8_t can have. */
#define INT_LEAST8_MIN     (-__INT_LEAST8_MAX__ - 1)

/** Largest value an \ref int_least8_t can have. */
#define UINT_LEAST8_MAX    __UINT_LEAST8_MAX__

/** Largest value an \ref int_least16_t can have. */
#define INT_LEAST16_MAX    __INT_LEAST16_MAX__

/** Smallest value an \ref int_least16_t can have. */
#define INT_LEAST16_MIN    (-__INT_LEAST16_MAX__ - 1)

/** Largest value an \ref int_least16_t can have. */
#define UINT_LEAST16_MAX   __UINT_LEAST16_MAX__

/** Largest value an \ref int_least32_t can have. */
#define INT_LEAST32_MAX    __INT_LEAST32_MAX__

/** Smallest value an \ref int_least32_t can have. */
#define INT_LEAST32_MIN    (-__INT_LEAST32_MAX__ - 1)

/** Largest value a \ref uint_least32_t can have. */
#define UINT_LEAST32_MAX   __UINT_LEAST32_MAX__

/** Largest value an \ref int_least64_t can have. */
#define INT_LEAST64_MAX    __INT_LEAST64_MAX__

/** Smallest value an \ref int_least64_t can have. */
#define INT_LEAST64_MIN    (-__INT_LEAST64_MAX__ - 1)

/** Largest value a \ref uint_least64_t can have. */
#define UINT_LEAST64_MAX   __UINT_LEAST64_MAX__

/** Largest value an \ref int_fast8_t can have. */
#define INT_FAST8_MAX      __INT_FAST8_MAX__

/** Smallest value an \ref int_fast8_t can have. */
#define INT_FAST8_MIN      (-__INT_FAST8_MAX__ - 1)

/** Largest value a \ref uint_fast8_t can have. */
#define UINT_FAST8_MAX     __UINT_FAST8_MAX__

/** Largest value an \ref int_fast16_t can have. */
#define INT_FAST16_MAX     __INT_FAST16_MAX__

/** Smallest value an \ref int_fast16_t can have. */
#define INT_FAST16_MIN     (-__INT_FAST16_MAX__ - 1)

/** Largest value a \ref uint_fast16_t can have. */
#define UINT_FAST16_MAX    __UINT_FAST16_MAX__

/** Largest value an \ref int_fast32_t can have. */
#define INT_FAST32_MAX     __INT_FAST32_MAX__

/** Smallest value an \ref int_fast32_t can have. */
#define INT_FAST32_MIN     (-__INT_FAST32_MAX__ - 1)

/** Largest value a \ref int_fast32_t can have. */
#define UINT_FAST32_MAX    __UINT_FAST32_MAX__

/** Largest value an \ref int_fast64_t can have. */
#define INT_FAST64_MAX     __INT_FAST64_MAX__

/** Smallest value an \ref int_fast64_t can have. */
#define INT_FAST64_MIN     (-__INT_FAST64_MAX__ - 1)

/** Largest value a \ref int_fast64_t can have. */
#define UINT_FAST64_MAX    __UINT_FAST64_MAX__

/** Largest value an \ref intptr_t can have. */
#define INTPTR_MAX         __INTPTR_MAX__

/** Smallest value an \ref intptr_t can have. */
#define INTPTR_MIN         (-__INTPTR_MAX__ - 1)

/** Largest value a \ref uintptr_t can have. */
#define UINTPTR_MAX        __UINTPTR_MAX__

/** Largest value an \ref intmax_t can have. */
#define INTMAX_MAX         __INTMAX_MAX__

/** Smallest value an \ref intmax_t can have. */
#define INTMAX_MIN         (-__INTMAX_MAX__ - 1)

/** Largest value a \ref uintmax_t can have. */
#define UINTMAX_MAX        __UINTMAX_MAX__

/** Width of \ref int8_t. */
#define INT8_WIDTH         8

/** Width of \ref int16_t. */
#define INT16_WIDTH        16

/** Width of \ref int32_t. */
#define INT32_WIDTH        32

/** Width of \ref int64_t. */
#define INT64_WIDTH        64

/** Width of \ref uint8_t. */
#define UINT8_WIDTH        8

/** Width of \ref uint16_t. */
#define UINT16_WIDTH       16

/** Width of \ref uint32_t. */
#define UINT32_WIDTH       32

/** Width of \ref uint64_t. */
#define UINT64_WIDTH       64

/** Width of \ref int_fast8_t. */
#define INT_FAST8_WIDTH    __INT_FAST8_WIDTH__

/** Width of \ref int_fast16_t. */
#define INT_FAST16_WIDTH   __INT_FAST16_WIDTH__

/** Width of \ref int_fast32_t. */
#define INT_FAST32_WIDTH   __INT_FAST32_WIDTH__

/** Width of \ref int_fast64_t. */
#define INT_FAST64_WIDTH   __INT_FAST64_WIDTH__

/** Width of \ref uint_fast8_t. */
#define UINT_FAST8_WIDTH   __INT_FAST8_WIDTH__

/** Width of \ref uint_fast16_t. */
#define UINT_FAST16_WIDTH  __INT_FAST16_WIDTH__

/** Width of \ref uint_fast32_t. */
#define UINT_FAST32_WIDTH  __INT_FAST32_WIDTH__

/** Width of \ref uint_fast64_t. */
#define UINT_FAST64_WIDTH  __INT_FAST64_WIDTH__

/** Width of \ref int_least8_t. */
#define INT_LEAST8_WIDTH   __INT_LEAST8_WIDTH__

/** Width of \ref int_least16_t. */
#define INT_LEAST16_WIDTH  __INT_LEAST16_WIDTH__

/** Width of \ref int_least32_t. */
#define INT_LEAST32_WIDTH  __INT_LEAST32_WIDTH__

/** Width of \ref int_least64_t. */
#define INT_LEAST64_WIDTH  __INT_LEAST64_WIDTH__

/** Width of \ref uint_least8_t. */
#define UINT_LEAST8_WIDTH  __INT_LEAST8_WIDTH__

/** Width of \ref uint_least16_t. */
#define UINT_LEAST16_WIDTH __INT_LEAST16_WIDTH__

/** Width of \ref uint_least32_t. */
#define UINT_LEAST32_WIDTH __INT_LEAST32_WIDTH__

/** Width of \ref uint_least64_t. */
#define UINT_LEAST64_WIDTH __INT_LEAST64_WIDTH__

/** Width of \ref intptr_t. */
#define INTPTR_WIDTH       __INTPTR_WIDTH__

/** Width of \ref intmax_t. */
#define INTMAX_WIDTH       __INTMAX_WIDTH__

/** Width of \ref uintptr_t. */
#define UINTPTR_WIDTH      __INTPTR_WIDTH__

/** Width of \ref uintmax_t. */
#define UINTMAX_WIDTH      __INTMAX_WIDTH__

/** Null. */
#define NULL               0

/* some common types used throughout the kernel */
/** Status, used with codes in \ref status_codes. */
typedef int_fast8_t stat_t;

/** ID of something. */
typedef uint_fast32_t id_t;

/** Memory region flags. */
typedef uint_fast16_t vmflags_t;

/**
 * Status codes.
 * Negative error codes are reserved for general usage, positive error codes are
 * allowed to be function-specific, although that's sort of difficult to keep
 * track of.
 */
/* should this enum be somewhere else? */
enum status_codes {
	/** Something went wrong :/ */
	ERR_MISC = -8,
	/** Not initialized. */
	ERR_NOINIT = -7,
	/** Invalid value. */
	ERR_INVAL = -6,
	/** Already exists. */
	ERR_EXT = -5,
	/** Out of memory. */
	ERR_OOMEM = -4,
	/** Illegal address. */
	ERR_ADDR = -3,
	/** Wrong alignment. */
	ERR_ALIGN = -2,
	/** Not found. */
	ERR_NF = -1,
	/** OK. */
	OK = 0,
	/** Try again. */
	INFO_TRGN = 1,
	/** Side effects. */
	INFO_SEFF = 2,
	/** Continue. */
	INFO_CONT = 3,
};

#include <arch/types.h> /* arch-specific type definitions (pm_t/vm_t etc) */

#endif /* APOS_TYPES_H */
