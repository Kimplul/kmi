#ifndef APOS_BITS_H
#define APOS_BITS_H

#include <apos/types.h>
#include <apos/builtin.h>

#define __is_set(x, y)     ((x) & (y))
#define __set_bit(x, y)    ((x) |= (y))
#define __clear_bit(x, y)  ((x) &= ~(y))

#define __is_nset(x, y)    (__is_set((x), 1UL << (y)))
#define __set_nbit(x, y)   (__set_bit((x), 1UL << (y)))
#define __clear_nbit(x, y) (__clear_bit((x), 1UL << (y)))

static inline bool bitmap_is_set(void *bmap, size_t n)
{
	uint8_t *bitmap = bmap;
	size_t i = n / 8;
	size_t r = n - (i * 8);
	return __is_nset(bitmap[i], r);
}

static inline void bitmap_set(void *bmap, size_t n)
{
	uint8_t *bitmap = bmap;
	size_t i = n / 8;
	size_t r = n - (i * 8);
	__set_nbit(bitmap[i], r);
}

static inline void bitmap_clear(void *bmap, size_t n)
{
	uint8_t *bitmap = bmap;
	size_t i = n / 8;
	size_t r = n - (i * 8);
	__clear_nbit(bitmap[i], r);
}

uint16_t __bswap16(uint16_t u);
uint32_t __bswap32(uint32_t u);
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
#define be16_to_cpu(x) __bswap16(x)
#define be32_to_cpu(x) __bswap32(x)
#define be64_to_cpu(x) __bswap64(x)

#define cpu_to_be16(x) __bswap16(x)
#define cpu_to_be32(x) __bswap32(x)
#define cpu_to_be64(x) __bswap64(x)

#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#define le64_to_cpu(x) (x)

#define cpu_to_le16(x) (x)
#define cpu_to_le32(x) (x)
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
