/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_DEBUG_H
#define APOS_DEBUG_H

/**
 * @file debug.h
 * Debug printing.
 *
 * Note that 'Integer format specifier' means "%i", which is identical to "%d"
 * in the context of outputting integers.
 */

#include <apos/attrs.h>
#include <apos/pmem.h>
#include <arch/vmem.h>

/** @name Internal. */
/** @{ */

/** Long integer prefix. Approximate, but probably good enough */
#if _LP64
#define __PRI64_PREFIX "l"
#else
#define __PRI64_PREFIX "ll"
#endif

/** Long pointer in integer type prefix. */
#if _LP64
#define __PRIPTR_PREFIX "l"
#else
#define __PRIPTR_PREFIX
#endif

/** @} */

/** @name Decimal format specifiers. */
/** @{ */

/** Decimal format specifief for \ref int8_t */
#define PRId8 "d"

/** Decimal format specifier for \ref int16_t */
#define PRId16 "d"

/** Decimal format specifier for \ref int32_t */
#define PRId32 "d"

/** Decimal format specifier for int16_t */
#define PRId64 __PRI64_PREFIX "d"

/** Decimal format specifier for \ref int_least8_t. */
#define PRIdLEAST8 "d"

/** Decimal format specifier for \ref int_least16_t. */
#define PRIdLEAST16 "d"

/** Decimal format specifier for \ref int_least32_t. */
#define PRIdLEAST32 "d"

/** Decimal format specifier for \ref int_least64_t. */
#define PRIdLEAST64 __PRI64_PREFIX "d"

/** Decimal format specifier for \ref int_fast8_t. */
#define PRIdFAST8 "d"

/** Decimal format specifier for \ref int_fast16_t. */
#define PRIdFAST16 __PRIPTR_PREFIX "d"

/** Decimal format specifier for \ref int_fast32_t. */
#define PRIdFAST32 __PRIPTR_PREFIX "d"

/** Decimal format specifier for \ref int_fast64_t. */
#define PRIdFAST64 __PRI64_PREFIX "d"

/** Decimal format specifier for \ref intmax_t. */
#define PRIdMAX __PRI64_PREFIX "d"

/** Decimal format specifier for \ref intptr_t. */
#define PRIdPTR __PRIPTR_PREFIX "d"

/** @} */

/** @name Integer format specifiers. */
/** @{ */

/** Integer format specifier for \ref int8_t. */
#define PRIi8 "i"

/** Integer format specifier for \ref int16_t. */
#define PRIi16 "i"

/** Integer format specifier for \ref int32_t. */
#define PRIi32 "i"

/** Integer format specifier for \ref int64_t. */
#define PRIi64 __PRI64_PREFIX "i"

/** Integer format specifier for \ref int_least8_t. */
#define PRIiLEAST8 "i"

/** Integer format specifier for \ref int_least16_t. */
#define PRIiLEAST16 "i"

/** Integer format specifier for \ref int_least32_t. */
#define PRIiLEAST32 "i"

/** Integer format specifier for \ref int_least64_t. */
#define PRIiLEAST64 __PRI64_PREFIX "i"

/** Integer format specifier for \ref int_fast8_t. */
#define PRIiFAST8 "i"

/** Integer format specifier for \ref int_fast16_t. */
#define PRIiFAST16 __PRIPTR_PREFIX "i"

/** Integer format specifier for \ref int_fast32_t. */
#define PRIiFAST32 __PRIPTR_PREFIX "i"

/** Integer format specifier for \ref int_fast64_t. */
#define PRIiFAST64 __PRI64_PREFIX "i"

/** Integer format specifier for \ref intmax_t. */
#define PRIiMAX __PRI64_PREFIX "i"

/** Integer format specifier for \ref intptr_t. */
#define PRIiPTR __PRIPTR_PREFIX "i"

/** @} */

/** @name Octal format specifiers. */
/** @{ */

/** Octal format specifier for \ref int8_t. */
#define PRIo8 "o"

/** Octal format specifier for \ref int16_t. */
#define PRIo16 "o"

/** Octal format specifier for \ref int32_t. */
#define PRIo32 "o"

/** Octal format specifier for \ref int64_t. */
#define PRIo64 __PRI64_PREFIX "o"

/** Octal format specifier for \ref int_least8_t. */
#define PRIoLEAST8 "o"

/** Octal format specifier for \ref int_least16_t. */
#define PRIoLEAST16 "o"

/** Octal format specifier for \ref int_least32_t. */
#define PRIoLEAST32 "o"

/** Octal format specifier for \ref int_least64_t. */
#define PRIoLEAST64 __PRI64_PREFIX "o"

/** Octal format specifier for \ref int_fast8_t. */
#define PRIoFAST8 "o"

/** Octal format specifier for \ref int_fast16_t. */
#define PRIoFAST16 __PRIPTR_PREFIX "o"

/** Octal format specifier for \ref int_fast32_t. */
#define PRIoFAST32 __PRIPTR_PREFIX "o"

/** Octal format specifier for \ref int_fast64_t. */
#define PRIoFAST64 __PRI64_PREFIX "o"

/** Octal format specifier for \ref uintmax_t. */
#define PRIoMAX __PRI64_PREFIX "o"

/** Octal format specifier for \ref uintptr_t. */
#define PRIoPTR __PRIPTR_PREFIX "o"

/** @} */

/** @name Unsigned decimal format specifiers. */
/** @{ */

/** Unsigned decimal format specifier for \ref uint8_t. */
#define PRIu8 "u"

/** Unsigned decimal format specifier for \ref uint16_t. */
#define PRIu16 "u"

/** Unsigned decimal format specifier for \ref uint32_t. */
#define PRIu32 "u"

/** Unsigned decimal format specifier for \ref uint64_t. */
#define PRIu64 __PRI64_PREFIX "u"

/** Unsigned decimal format specifier for \ref uint_least8_t. */
#define PRIuLEAST8 "u"

/** Unsigned decimal format specifier for \ref uint_least16_t. */
#define PRIuLEAST16 "u"

/** Unsigned decimal format specifier for \ref uint_least32_t. */
#define PRIuLEAST32 "u"

/** Unsigned decimal format specifier for \ref uint_least64_t. */
#define PRIuLEAST64 __PRI64_PREFIX "u"

/** Unsigned decimal format specifier for \ref uint_fast8_t. */
#define PRIuFAST8 "u"

/** Unsigned decimal format specifier for \ref uint_fast16_t. */
#define PRIuFAST16 __PRIPTR_PREFIX "u"

/** Unsigned decimal format specifier for \ref uint_fast32_t. */
#define PRIuFAST32 __PRIPTR_PREFIX "u"

/** Unsigned decimal format specifier for \ref uint_fast64_t. */
#define PRIuFAST64 __PRI64_PREFIX "u"

/** Unsigned decimal format specifier for \ref uintmax_t. */
#define PRIuMAX __PRI64_PREFIX "u"

/** Unsigned decimal format specifier for \ref uintptr_t. */
#define PRIuPTR __PRIPTR_PREFIX "u"

/** @} */

/** @name Hex format specifiers. */
/** @{ */

/** Hex format specifier for \ref uint8_t. */
#define PRIx8 "x"

/** Hex format specifier for \ref uint16_t. */
#define PRIx16 "x"

/** Hex format specifier for \ref uint32_t. */
#define PRIx32 "x"

/** Hex format specifier for \ref uint64_t. */
#define PRIx64 __PRI64_PREFIX "x"

/** Hex format specifier for \ref uint_least8_t. */
#define PRIxLEAST8 "x"

/** Hex format specifier for \ref uint_least16_t. */
#define PRIxLEAST16 "x"

/** Hex format specifier for \ref uint_least32_t. */
#define PRIxLEAST32 "x"

/** Hex format specifier for \ref uint_least64_t. */
#define PRIxLEAST64 __PRI64_PREFIX "x"

/** Hex format specifier for \ref uint_fast8_t. */
#define PRIxFAST8 "x"

/** Hex format specifier for \ref uint_fast16_t. */
#define PRIxFAST16 __PRIPTR_PREFIX "x"

/** Hex format specifier for \ref uint_fast32_t. */
#define PRIxFAST32 __PRIPTR_PREFIX "x"

/** Hex format specifier for \ref uint_fast64_t. */
#define PRIxFAST64 __PRI64_PREFIX "x"

/** Hex format specifier for \ref uintmax_t. */
#define PRIxMAX __PRI64_PREFIX "x"

/** Hex format specifier for \ref uintptr_t. */
#define PRIxPTR __PRIPTR_PREFIX "x"

/** @} */

/** @name Binary notation  */
/** @{ */

/** Binary format specifier for \ref uint8_t. */
#define PRIX8 "X"

/** Binary format specifier for \ref uint16_t. */
#define PRIX16 "X"

/** Binary format specifier for \ref uint32_t. */
#define PRIX32 "X"

/** Binary format specifier for \ref uint64_t. */
#define PRIX64 __PRI64_PREFIX "X"

/** Binary format specifier for \ref uint_least8_t. */
#define PRIXLEAST8 "X"

/** Binary format specifier for \ref uint_least16_t. */
#define PRIXLEAST16 "X"

/** Binary format specifier for \ref uint_least32_t. */
#define PRIXLEAST32 "X"

/** Binary format specifier for \ref uint_least64_t. */
#define PRIXLEAST64 __PRI64_PREFIX "X"

/** Binary format specifier for \ref uint_fast8_t. */
#define PRIXFAST8 "X"

/** Binary format specifier for \ref uint_fast16_t. */
#define PRIXFAST16 __PRIPTR_PREFIX "X"

/** Binary format specifier for \ref uint_fast32_t. */
#define PRIXFAST32 __PRIPTR_PREFIX "X"

/** Binary format specifier for \ref uint_fast64_t. */
#define PRIXFAST64 __PRI64_PREFIX "X"

/** Binary format specifier for \ref uintmax_t. */
#define PRIXMAX __PRI64_PREFIX "X"

/** Binary format specifier for \ref uintptr_t. */
#define PRIXPTR __PRIPTR_PREFIX "X"

/** @} */

#if defined(DEBUG)
/** Serial devices supported. */
enum serial_dev {
	/** NS16550A and compatible. Currently the only supported serial device. */
	NS16550A,
};

/**
 * dbg, apos equivalent of printf.
 *
 * @param fmt Format string. Integer subset of normal printf formatting.
 */
void __printf dbg(const char *fmt, ...);

/**
 * Initialize debugging, set up serial driver etc.
 *
 * @param fdt Pointer to global FDT.
 */
void init_dbg(const void *fdt);

/**
 * Setup debugging in direct mapping context.
 */
void setup_dmap_dbg();

/**
 * Setup debugging in virtual memory context.
 *
 * @param vmem Virtual memory space to use.
 */
void setup_io_dbg(struct vmem *vmem);

/** @name Internal. */
/** @{ */

/**
 * Format to append to helper debugging classes.
 * See \ref bug(), \ref warn(), \ref info() and \ref error().
 */
#define COMMON_FORMAT "[%s] %s:%d\n\t"

/**
 * Helper for helper classes.
 * See \ref bug(), \ref warn(), \ref info(), \ref error().
 */
#define COMMON_ARGS(s) s, __FILE__, __LINE__

/** @} */

/**
 * Print a bug message to the serial lines.
 *
 * @param fmt Message, integer subset of regular printf.
 */
#define bug(fmt, ...) dbg(COMMON_FORMAT fmt, COMMON_ARGS("BUG"),##__VA_ARGS__)

/**
 * Print a warning message to the serial lines.
 *
 * @param fmt Message, integer subset of regular printf.
 */
#define warn(fmt, ...) dbg(COMMON_FORMAT fmt, COMMON_ARGS("WARN"),##__VA_ARGS__)

/**
 * Print an informational message to the serial lines.
 *
 * @param fmt Message, integer subset of regular printf.
 */
#define info(fmt, ...) dbg(COMMON_FORMAT fmt, COMMON_ARGS("INFO"),##__VA_ARGS__)

/**
 * Prnt an error message to the serial lines.
 *
 * @param fmt Message, integer subset of regular printf.
 */
#define error(fmt, ...) \
	dbg(COMMON_FORMAT fmt, COMMON_ARGS("ERROR"),##__VA_ARGS__)

#else

/* already documented in previous #if block, let's hope Doxygen doesn't start
 * complaining :) */
#define dbg(...)
#define dbg_init(...)
#define dbg_from_fdt(...)

#define init_dbg(...)
#define setup_dmap_dbg(...)
#define setup_io_dbg(...)

#define bug(...)
#define warn(...)
#define info(...)
#define error(...)

#endif /* DEBUG */

#endif /* APOS_DEBUG_H */
