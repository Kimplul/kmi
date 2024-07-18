/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file bits.c
 * Bit manipulation helper implementations, currently just byte swaps.
 */

#include <kmi/types.h>
#include <kmi/attrs.h>
#include <kmi/bits.h>
#include <kmi/builtin.h>

#undef __bswap16
__weak uint16_t __bswap16(const uint16_t u)
{
	return (u & 0xff00) >> 8 | (u & 0x00ff) << 8;
}

/**
 * Wrapper for gcc builtins, if arch doesn't have it.
 *
 * @param u Value to byteswap.
 * @return \p u byteswapped.
 */
__used uint16_t __bswaphi2(uint16_t u)
{
	return __bswap16(u);
}

#undef __bswap32
__weak uint32_t __bswap32(const uint32_t u)
{
	return (u & 0xff000000) >> 24 | (u & 0x00ff0000) >> 8 |
	       (u & 0x0000ff00) << 8 | (u & 0x000000ff) << 24;
}

/**
 * Wrapper for gcc builtins, if arch doesn't have it.
 *
 * @param u Value to byteswap.
 * @return \p u byteswapped.
 */
__used uint32_t __bswapsi2(uint32_t u)
{
	return __bswap32(u);
}

#undef __bswap64
__weak uint64_t __bswap64(const uint64_t u)
{
	return (u & 0xff00000000000000ULL) >> 56 |
	       (u & 0x00ff000000000000ULL) >> 40 |
	       (u & 0x0000ff0000000000ULL) >> 24 |
	       (u & 0x000000ff00000000ULL) >> 8 |
	       (u & 0x00000000ff000000ULL) << 8 |
	        (u & 0x0000000000ff0000ULL) << 24 |
	        (u & 0x000000000000ff00ULL) << 40 |
	        (u & 0x00000000000000ffULL) << 56;
}

/**
 * Wrapper for gcc builtins, if arch doesn't have it.
 *
 * @param u Value to byteswap.
 * @return \p u byteswapped.
 */
__used uint64_t __bswapdi2(uint64_t u)
{
	return __bswap64(u);
}

#undef ffs
__weak int ffs(int v)
{
	/* http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel */
	if (v == 0)
		return 0;

	/* silence ubsan warning */
	if (v == INT_MIN)
		return 32;

	int c = 32;
	v &= -v;

	if (v) c--;
	if (v & 0x0000FFFF) c -= 16;
	if (v & 0x00FF00FF) c -= 8;
	if (v & 0x0F0F0F0F) c -= 4;
	if (v & 0x33333333) c -= 2;
	if (v & 0x55555555) c -= 1;

	return c + 1;
}
