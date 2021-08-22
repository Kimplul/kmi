#include <apos/types.h>
#include <apos/attrs.h>

/* Functions for swapping endianness.
 *
 * Typically you wouldn't call these directly, instead using macros such as
 * le16_to_cpu, cpu_to_be32, etc. that take the system endianness into account.
 *
 * The macros cpu_to_be64 etc. use GCC's and Clang's __builtin_bswap16/32/64, but in
 * the case that the arch compiler doesn't provide the __builtin functions they're replaced
 * with __bswaphi2/si2/di2 respectively, and it is up to the developer to
 * provide these. The naming scheme uses GCC's internal naming scheme, where
 *
 * HI ~= 16 bits
 * SI ~= 32 bits
 * DI ~= 64 bits
 *
 * Here we assume this to be true in all cases, and it probably is, but in the
 * case that some architecture is different (SI ~= 64 bits or something) we define
 * these functions as __weak, allowing any arch to redefine them.
 */

__weak uint16_t __bswaphi2(uint16_t u)
{
	return (u & 0xff00) >> 8 | (u & 0x00ff) << 8;
}

__weak uint32_t __bswapsi2(uint32_t u)
{
	return (u & 0xff000000) >> 24 |
		(u & 0x00ff0000) >> 8 |
		(u & 0x0000ff00) << 8 |
		(u & 0x000000ff) << 24;
}

__weak uint64_t __bswapdi2(uint64_t u)
{
	return (u & 0xff00000000000000ULL) >> 56	|
		(u & 0x00ff000000000000ULL) >> 40	|
		(u & 0x0000ff0000000000ULL) >> 24	|
		(u & 0x000000ff00000000ULL) >> 8	|
		(u & 0x00000000ff000000ULL) << 8	|
		(u & 0x0000000000ff0000ULL) << 24	|
		(u & 0x000000000000ff00ULL) << 40	|
		(u & 0x00000000000000ffULL) << 56;
}
