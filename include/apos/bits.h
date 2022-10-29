/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_BITS_H
#define APOS_BITS_H

/**
 * @file bits.h
 * Bit manipulations.
 */

#include <apos/types.h>
#include <apos/builtin.h>

/** @name Arithmetic integer bit manipulation. */
/** @{ */
/**
 * Find first set bit in \c int.
 *
 * @param v Integer to find first set bit in.
 * @return Index of least significant bit + 1 or 0 if \p v is 0.
 */
#if __has_builtin(__builtin_ffs)
#define ffs(v) __builtin_ffs(v)
#else
int ffs(int v);
#endif

/**
 * Check if bits are set.
 *
 * @param x Value to check in.
 * @param y Mask of bits to check.
 * @return \c 0 if none of the bits aren't set, non-zero otherwise.
 */
#define is_set(x, y) ((x) & (y))

/**
 * Set bits.
 *
 * @param x Value to set in.
 * @param y Mask of bits to set.
 */
#define set_bits(x, y) ((x) |= (y))

/**
 * Clear bits.
 *
 * @param x Value to clear in.
 * @param y Mask of bits to clear.
 */
#define clear_bits(x, y) ((x) &= ~(y))

/**
 * Set bit.
 * Wrapper around \ref set_bits() for when only one bit is changed,
 * mostly just for readability purposes.
 *
 * @param x Value to set in.
 * @param y Mask of bit to set.
 */
#define set_bit(x, y) set_bits(x, y)

/** Clear bit.
 * Wrapper around \ref clear_bits() for when only one bit is
 * changed, mostly just for readability purposes.
 *
 * @param x Value to clear in.
 * @param y Mask of bit to clear.
 */
#define clear_bit(x, y) clear_bits(x, y)

/**
 * Is nth bit set.
 *
 * @param x Value to check in.
 * @param y Index of bit to check.
 * @return \c 0 if bit is not set, non-zero otherwise.
 */
#define is_nset(x, y) (is_set((x), 1UL << (y)))

/**
 * Set nth bit.
 *
 * @param x Value to set in.
 * @param y Index of bit to set.
 */
#define set_nbit(x, y) (set_bit((x), 1UL << (y)))

/** Clear nth bit.
 *
 * @param x Value to clear in.
 * @param y Index of bit to clear.
 */
#define clear_nbit(x, y) (clear_bit((x), 1UL << (y)))

/** @} */

/**
 * @name Bitmap manipulation.
 * A bitmap can be any number of bits in an array-like structure.
 */
/** @{ */

/**
 * Is nth bit set in bitmap.
 *
 * @param bmap Pointer to bitmap.
 * @param n Index of bit to check.
 * @return \c 0 if bit is not set, non-zero otherwise.
 */
static inline bool bitmap_is_set(void *bmap, size_t n)
{
	uint8_t *bitmap = bmap;
	size_t i = n / 8;
	size_t r = n - (i * 8);
	return is_nset(bitmap[i], r);
}

/**
 * Set nth bit in bitmap.
 *
 * @param bmap Bitmap.
 * @param n Index of bit to set.
 */
static inline void bitmap_set(void *bmap, size_t n)
{
	uint8_t *bitmap = bmap;
	size_t i = n / 8;
	size_t r = n - (i * 8);
	set_nbit(bitmap[i], r);
}

/**
 * Clear nth bit in bitmap.
 *
 * @param bmap Bitmap.
 * @param n Index of bit to clear.
 */
static inline void bitmap_clear(void *bmap, size_t n)
{
	uint8_t *bitmap = bmap;
	size_t i = n / 8;
	size_t r = n - (i * 8);
	clear_nbit(bitmap[i], r);
}

/**
 * Find first bit, either set or unset, in bitmap.
 *
 * @param bmap Bitmap.
 * @param n Size of bitmap in bits.
 * @param set Wether to seek for set or unset bits.
 * @return Index of found bit + 1 or \p n + 1 if no bit was found.
 */
static inline size_t bitmap_find_first(void *bmap, size_t n, bool set)
{
	size_t i = n / (sizeof(int) * 8);

	size_t c = 0;
	int *imap = (int *)bmap;

	int target = set ? 0 : -1;
	for (; c < i; ++c)
		if (imap[c] != target)
			break;

	size_t b = c * sizeof(int) * 8;
	int check = set ? imap[c] : ~imap[c];
	if (c != i)
		return b + ffs(check) - 1;

	size_t r = n - (i * sizeof(int) * 8);
	if (!r)
		return n + 1;

	bool comp = set ? true : false;
	for (size_t a = 0; a < r; ++a)
		if (bitmap_is_set(bmap, b + a) == comp)
			return b + a;

	return n + 1;
}

/**
 * Convenience wrapper around bitmap_find_first().
 *
 * @param bmap \see bitmap_find_first().
 * @param n \see bitmap_find_first().
 * @return \see bitmap_find_first().
 */
static inline size_t bitmap_find_first_unset(void *bmap, size_t n)
{
	return bitmap_find_first(bmap, n, false);
}

/**
 * Convenience wrapper around bitmap_find_first().
 *
 * @param bmap \see bitmap_find_first().
 * @param n \see bitmap_find_first().
 * @return \see bitmap_find_first().
 */
static inline size_t bitmap_find_first_set(void *bmap, size_t n)
{
	return bitmap_find_first(bmap, n, true);
}

/** @} */

/**
 * Swap byte order in \ref uint16_t.
 *
 * @param u \ref uint16_t to swap.
 * @return \c u with its byte order swapper.
 */
uint16_t __bswap16(uint16_t u);

/**
 * Swap byte order in \ref uint32_t.
 *
 * @param u \ref uint32_t to swap.
 * @return \c u with its byte order swapped.
 */
uint32_t __bswap32(uint32_t u);

/**
 * Swap byte order in \ref uint64_t.
 *
 * @param u \ref uint64_t to swap.
 * @return \c u with its byte order swapped.
 */
uint64_t __bswap64(uint64_t u);

#if __has_builtin(__builtin_bswap16)
#define __bswap16(x) __builtin_bswap16(x)
#endif

#if __has_builtin(__builtin_bswap32)
#define __bswap32(x) __builtin_bswap32(x)
#endif

#if __has_builtin(__builtin_bswap64)
#define __bswap64(x) __builtin_bswap64(x)
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

/**
 * Convert big endian \ref uint16_t to cpu endianness.
 *
 * @param x \ref uint16_t to convert.
 * @return \c x in cpu endianness.
 */
#define be16_to_cpu(x) __bswap16(x)

/**
 * Convert big endian \ref uint32_t to cpu endianness.
 *
 * @param x \ref uint32_t to convert.
 * @return \c x in cpu endianness.
 */
#define be32_to_cpu(x) __bswap32(x)

/**
 * Convert big endian \ref uint64_t to cpu endianness.
 *
 * @param x \ref uint64_t to convert.
 * @return \c x in cpu endianness.
 */
#define be64_to_cpu(x) __bswap64(x)

/**
 * Convert cpu endian \ref uint16_t to big endian.
 *
 * @param x \ref uint16_t to convert.
 * @return \c x in big endian.
 */
#define cpu_to_be16(x) __bswap16(x)

/**
 * Convert cpu endian \ref uint32_t to big endian.
 *
 * @param x \ref uint32_t to convert.
 * @return \c x in big endian.
 */
#define cpu_to_be32(x) __bswap32(x)

/**
 * Convert cpu endian to \ref uint64_t to big endian.
 *
 * @param x \ref uint64_t to convert.
 * @return \c x in big endian.
 */
#define cpu_to_be64(x) __bswap64(x)

/**
 * Convert little endian \ref uint16_t to cpu endianness.
 *
 * @param x \ref uint16_t to convert.
 * @return \c x in cpu endianness.
 */
#define le16_to_cpu(x) (x)

/**
 * Convert little endian \ref uint32_t to cpu endianness.
 *
 * @param x \ref uint32_t to convert.
 * @return \c x in cpu endianness.
 */
#define le32_to_cpu(x) (x)

/**
 * Convert little endian \ref uint64_t to cpu endianness.
 *
 * @param x \ref uint64_t to convert.
 * @return \c x in cpu endianness.
 */
#define le64_to_cpu(x) (x)

/**
 * Convert cpu endian \ref uint16_t to little endian.
 *
 * @param x \ref uint16_t to convert.
 * @return \c x in little endian.
 */
#define cpu_to_le16(x) (x)

/**
 * Convert cpu endian \ref uint32_t to little endian.
 *
 * @param x \ref uint32_t to convert.
 * @return \c x in little endian.
 */
#define cpu_to_le32(x) (x)

/** Convert cpu endian \ref uint64_t to little endian.
 *
 * @param x \ref uint64_t to convert.
 * @return \c x in little endian.
 */
#define cpu_to_le64(x) (x)

#else

#define be16_to_cpu(x) (x)
#define be32_to_cpu(x) (x)
#define be64_to_cpu(x) (x)

#define cpu_to_be16(x) (x)
#define cpu_to_be32(x) (x)
#define cpu_to_be64(x) (x)

#define le16_to_cpu(x) __bswap16(x)
#define le32_to_cpu(x) __bswap32(x)
#define le64_to_cpu(x) __bswap64(x)

#define cpu_to_le16(x) __bswap16(x)
#define cpu_to_le32(x) __bswap32(x)
#define cpu_to_le64(x) __bswap64(x)

#endif

#endif /* APOS_BITS_H */
