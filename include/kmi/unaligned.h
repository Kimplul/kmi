/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_UNALIGNED_H
#define KMI_UNALIGNED_H

/**
 * @file unaligned.h
 * Helpers for unaligned memory accesses. Largely lifted from Linux.
 */

#include <kmi/types.h>
#include <kmi/attrs.h>

/**
 * Get unaligned value. Type of value is deduced from pointer type.
 *
 * @param ptr Pointer to possibly unaligned value to read.
 * @return Value pointed to by \c ptr.
 */
#define get_unaligned(ptr)                         \
	_Generic(*(ptr),                           \
	         uint8_t: get_unaligned_uint8_t,   \
	         uint16_t: get_unaligned_uint16_t, \
	         uint32_t: get_unaligned_uint32_t, \
	         uint64_t: get_unaligned_uint64_t, \
                                                   \
	         int8_t: get_unaligned_int8_t,     \
	         int16_t: get_unaligned_int16_t,   \
	         int32_t: get_unaligned_int32_t,   \
	         int64_t: get_unaligned_int64_t)((void *)ptr)

/**
 * Put unaligned value. Type of value is deduced from pointer type.
 *
 * @param val Value to write to memory.
 * @param ptr Pointer to possibly unaligned address.
 */
#define put_unaligned(val, ptr)                    \
	_Generic(*(ptr),                           \
	         uint8_t: put_unaligned_uint8_t,   \
	         uint16_t: put_unaligned_uint16_t, \
	         uint32_t: put_unaligned_uint32_t, \
	         uint64_t: put_unaligned_uint64_t, \
                                                   \
	         int8_t: put_unaligned_int8_t,     \
	         int16_t: put_unaligned_int16_t,   \
	         int32_t: put_unaligned_int32_t,   \
	         int64_t: put_unaligned_int64_t)(val, (void *)ptr)

/**
 * Helper macro for defining an unaligned value reader.
 *
 * @param type Type of value reader to define.
 */
#define DEFINE_GET(type)                                   \
	static inline type get_unaligned_##type(void *ptr) \
	{                                                  \
		const struct __packed {                    \
			type x;                            \
		} *__pptr = ptr;                           \
		return __pptr->x;                          \
	}

/**
 * Read possibly unaligned \ref uint8_t.
 *
 * Technically speaking a byte can't be unaligned, but this is just here for
 * cohesion.
 *
 * @warning Prefer using \ref get_unaligned().
 *
 * @param ptr Pointer to \ref uint8_t to read.
 * @return Value pointed to by \c ptr.
 */
DEFINE_GET(uint8_t);

/**
 * Read possibly unaligned \ref uint16_t.
 *
 * @warning Prefer using \ref get_unaligned().
 *
 * @param ptr Pointer to \ref uint16_t to read.
 * @return Value pointed to by \c ptr.
 */
DEFINE_GET(uint16_t);

/**
 * Read possibly unaligned \ref uint32_t.
 *
 * @warning Prefer using \ref get_unaligned().
 *
 * @param ptr Pointer to \ref uint32_t to read.
 * @return Value pointed to by \c ptr.
 */
DEFINE_GET(uint32_t);

/**
 * Read possibly unaligned \ref uint64_t.
 *
 * @warning Prefer using \ref get_unaligned().
 *
 * @param ptr Pointer to \ref uint64_t to read.
 * @return Value pointed to by \c ptr.
 */
DEFINE_GET(uint64_t);

/**
 * Read possibly unaligned \ref int8_t.
 *
 * Technically speaking a byte can't be unaligned, but this is just here for
 * cohesion.
 *
 * @warning Prefer using \ref get_unaligned().
 *
 * @param ptr Pointer to \ref int8_t to read.
 * @return Value pointed to by \c ptr.
 */
DEFINE_GET(int8_t);

/**
 * Read possibly unaligned \ref int16_t.
 *
 * @warning Prefer using \ref get_unaligned().
 *
 * @param ptr Pointer to \ref int16_t to read.
 * @return Value pointed to by \c ptr.
 */
DEFINE_GET(int16_t);

/**
 * Read possibly unaligned \ref int32_t.
 *
 * @warning Prefer using \ref get_unaligned().
 *
 * @param ptr Pointer to \ref int32_t to read.
 * @return Value pointed to by \c ptr.
 */
DEFINE_GET(int32_t);

/**
 * Read possibly unaligned \ref int64_t.
 *
 * @warning Prefer using \ref get_unaligned().
 *
 * @param ptr Pointer to \ref int64_t to read.
 * @return Value pointed to by \c ptr.
 */
DEFINE_GET(int64_t);

#undef DEFINE_GET

/**
 * Helper macro for defining an unaligned writer.
 *
 * @param type Type of value to write.
 */
#define DEFINE_PUT(type)                                             \
	static inline void put_unaligned_##type(type val, void *ptr) \
	{                                                            \
		struct __packed {                                    \
			type x;                                      \
		} *__pptr = ptr;                                     \
		__pptr->x = val;                                     \
	}

/**
 * Write possibly unaligned \ref uint8_t.
 *
 * Technically speaking a byte can't be unaligned, but this is just here for
 * cohesion.
 *
 * @warning Prefer using \ref put_unaligned().
 *
 * @param val Value to write.
 * @param ptr Address to write to.
 */
DEFINE_PUT(uint8_t);

/**
 * Write possibly unaligned \ref uint16_t.
 *
 * @warning Prefer using \ref put_unaligned().
 *
 * @param val Value to write.
 * @param ptr Address to write to.
 */
DEFINE_PUT(uint16_t);

/**
 * Write possibly unaligned \ref uint32_t.
 *
 * @warning Prefer using \ref put_unaligned().
 *
 * @param val Value to write.
 * @param ptr Address to write to.
 */
DEFINE_PUT(uint32_t);

/**
 * Write possibly unaligned \ref uint64_t.
 *
 * @warning Prefer using \ref put_unaligned().
 *
 * @param val Value to write.
 * @param ptr Address to write to.
 */
DEFINE_PUT(uint64_t);

/**
 * Write possibly unaligned \ref int8_t.
 *
 * Technically speaking a byte can't be unaligned, but this is just here for
 * cohesion.
 *
 * @warning Prefer using \ref put_unaligned().
 *
 * @param val Value to write.
 * @param ptr Address to write to.
 */
DEFINE_PUT(int8_t);

/**
 * Write possibly unaligned \ref int16_t.
 *
 * @warning Prefer using \ref put_unaligned().
 *
 * @param val Value to write.
 * @param ptr Address to write to.
 */
DEFINE_PUT(int16_t);

/**
 * Write possibly unaligned \ref int32_t.
 *
 * @warning Prefer using \ref put_unaligned().
 *
 * @param val Value to write.
 * @param ptr Address to write to.
 */
DEFINE_PUT(int32_t);

/**
 * Write possibly unaligned \ref int64_t.
 *
 * @warning Prefer using \ref put_unaligned().
 *
 * @param val Value to write.
 * @param ptr Address to write to.
 */
DEFINE_PUT(int64_t);

#undef DEFINE_PUT

#endif /* KMI_UNALIGNED_H */
