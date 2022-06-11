#ifndef LIBFDT_ENV_H
/* take over libfdt */
#define LIBFDT_ENV_H

/**
 * @file libfdt_env.h
 * Set up apos environment for libfdt.
 */

#include <apos/types.h>
#include <apos/string.h>
#include <apos/bits.h>

/** 16bit integer. */
typedef int16_t fdt16_t;

/** 32bit integer. */
typedef int32_t fdt32_t;

/** 64bit integer. */
typedef int64_t fdt64_t;

/**
 * Convert FDT 32bit integer to cpu endianness.
 *
 * @param x Integer to convert.
 * @return Value of \c x in cpu endianness.
 */
#define fdt32_to_cpu(x) be32_to_cpu(x)

/**
 * Convert 32bit cpu endian integer to FDT endianness.
 *
 * @param x Integer to convert.
 * @return Value of \c x in FDT endianness.
 */
#define cpu_to_fdt32(x) cpu_to_be32(x)

/**
 * Convert FDT 64bit integer to cpu endianness.
 *
 * @param x Integer to convert.
 * @return Value of \c x in cpu endianness.
 */
#define fdt64_to_cpu(x) be64_to_cpu(x)

/**
 * Convert 64bit cpu endian integer to FDT endianness.
 *
 * @param x Integer to convert.
 * @return Value of \c x in FDT endianness.
 */
#define cpu_to_fdt64(x) cpu_to_be64(x)

#endif /* LIBFDT_ENV_H */
