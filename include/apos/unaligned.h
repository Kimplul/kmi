#ifndef APOS_UNALIGNED_H
#define APOS_UNALIGNED_H

/**
 * @file unaligned.h
 * Helpers for unaligned memory accesses. Largely lifted from Linux.
 */

#include <apos/types.h>
#include <apos/attrs.h>

#define get_unaligned(ptr)                                                     \
	_Generic(*(ptr), uint8_t                                               \
	         : __get_unaligned_uint8_t, uint16_t                           \
	         : __get_unaligned_uint16_t, uint32_t                          \
	         : __get_unaligned_uint32_t, uint64_t                          \
	         : __get_unaligned_uint64_t, int8_t                            \
	         : __get_unaligned_int8_t, int16_t                             \
	         : __get_unaligned_int16_t, int32_t                            \
	         : __get_unaligned_int32_t, int64_t                            \
	         : __get_unaligned_int64_t)((void *)ptr)

#define put_unaligned(val, ptr)                                                \
	_Generic(*(ptr), uint8_t                                               \
	         : __put_unaligned_uint8_t, uint16_t                           \
	         : __put_unaligned_uint16_t, uint32_t                          \
	         : __put_unaligned_uint32_t, uint64_t                          \
	         : __put_unaligned_uint64_t, int8_t                            \
	         : __put_unaligned_int8_t, int16_t                             \
	         : __put_unaligned_int16_t, int32_t                            \
	         : __put_unaligned_int32_t, int64_t                            \
	         : __put_unaligned_int64_t)(val, (void *)ptr)

#define DEFINE_GET(type)                                                       \
	static inline type __get_unaligned_##type(void *ptr)                   \
	{                                                                      \
		const struct __packed {                                        \
			type x;                                                \
		} *__pptr = ptr;                                               \
		return __pptr->x;                                              \
	}

DEFINE_GET(uint8_t);
DEFINE_GET(uint16_t);
DEFINE_GET(uint32_t);
DEFINE_GET(uint64_t);
DEFINE_GET(int8_t);
DEFINE_GET(int16_t);
DEFINE_GET(int32_t);
DEFINE_GET(int64_t);

#undef DEFINE_GET

#define DEFINE_PUT(type)                                                       \
	static inline void __put_unaligned_##type(type val, void *ptr)         \
	{                                                                      \
		struct __packed {                                              \
			type x;                                                \
		} *__pptr = ptr;                                               \
		__pptr->x = val;                                               \
	}

DEFINE_PUT(uint8_t);
DEFINE_PUT(uint16_t);
DEFINE_PUT(uint32_t);
DEFINE_PUT(uint64_t);
DEFINE_PUT(int8_t);
DEFINE_PUT(int16_t);
DEFINE_PUT(int32_t);
DEFINE_PUT(int64_t);

#endif /* APOS_UNALIGNED_H */
