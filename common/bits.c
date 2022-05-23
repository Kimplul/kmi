/**
 * @file bits.c
 * Bit manipulation helper implementations, currently just byte swaps.
 */

#include <apos/types.h>
#include <apos/attrs.h>
#include <apos/bits.h>
#include <apos/builtin.h>

#undef __bswap16
__weak uint16_t __bswap16(const uint16_t u)
{
	return (u & 0xff00) >> 8 | (u & 0x00ff) << 8;
}

#undef __bswap32
__weak uint32_t __bswap32(const uint32_t u)
{
	return (u & 0xff000000) >> 24 | (u & 0x00ff0000) >> 8 |
	       (u & 0x0000ff00) << 8 | (u & 0x000000ff) << 24;
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
