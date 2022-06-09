#ifndef ATOMIC_H
#define ATOMIC_H

/**
 * @file atomic.h
 * Atomics, closely modeled after C17 stdatomic.h. Largely dependent on the
 * compiler at the moment, if I run into an architecture that required the OS'
 * help to implement these I'll have to look into how to do that, but for now
 * this is probably good enough.
 */

#include <apos/utils.h> /* GLUE */

/**
 * Memory ordering modes.
 *
 * @remark Look up details somewhere elsewhere, as I'm not an expert on memory ordering.
 * Would like to be, though.
 */
typedef enum {
	/** Relaxed memory ordering. */
	memory_order_relaxed = __ATOMIC_RELAXED,
	/** Consume memory ordering. */
	memory_order_consume = __ATOMIC_CONSUME,
	/** Acquire memory ordering. */
	memory_order_acquire = __ATOMIC_ACQUIRE,
	/** Release memory ordering. */
	memory_order_release = __ATOMIC_RELEASE,
	/** Acquire and release memory ordering. */
	memory_order_acq_rel = __ATOMIC_ACQ_REL,
	/** Sequential memory ordering. */
	memory_order_seq_cst = __ATOMIC_SEQ_CST
} memory_order;

/** Atomic boolean. */
typedef _Atomic _Bool atomic_bool;

/** Atomic char. */
typedef _Atomic char atomic_char;

/** Atomic signed char. */
typedef _Atomic signed char atomic_schar;

/** Atomic unsigned char. */
typedef _Atomic unsigned char atomic_uchar;

/** Atomic signed short. */
typedef _Atomic short atomic_short;

/** Atomic unsigned short. */
typedef _Atomic unsigned short atomic_ushort;

/** Atomic signed int. */
typedef _Atomic int atomic_int;

/** Atomic unsigned int. */
typedef _Atomic unsigned int atomic_uint;

/** Atomic signed long. */
typedef _Atomic long atomic_long;

/** Atomic unsigned long. */
typedef _Atomic unsigned long atomic_ulong;

/** Atomic signed long long. */
typedef _Atomic long long atomic_llong;

/** Atomic unsigned long long. */
typedef _Atomic unsigned long long atomic_ullong;

/** Atomic 16bit char. */
typedef _Atomic __CHAR16_TYPE__ atomic_char16_t;

/** Atomic 32bit char. */
typedef _Atomic __CHAR32_TYPE__ atomic_char32_t;

/** Atomic \ref wchar_t. */
typedef _Atomic __WCHAR_TYPE__ atomic_wchar_t;

/** Atomic \ref int_least8_t. */
typedef _Atomic __INT_LEAST8_TYPE__ atomic_int_least8_t;

/** Atomic \ref uint_least8_t. */
typedef _Atomic __UINT_LEAST8_TYPE__ atomic_uint_least8_t;

/** Atomic \ref int_least16_t. */
typedef _Atomic __INT_LEAST16_TYPE__ atomic_int_least16_t;

/** Atomic \ref uint_least16_t. */
typedef _Atomic __UINT_LEAST16_TYPE__ atomic_uint_least16_t;

/** Atomic \ref int_least32_t. */
typedef _Atomic __INT_LEAST32_TYPE__ atomic_int_least32_t;

/** Atomic \ref uint_least32_t. */
typedef _Atomic __UINT_LEAST32_TYPE__ atomic_uint_least32_t;

/** Atomic \ref int_least64_t. */
typedef _Atomic __INT_LEAST64_TYPE__ atomic_int_least64_t;

/** Atomic \ref uint_least64_t. */
typedef _Atomic __UINT_LEAST64_TYPE__ atomic_uint_least64_t;

/** Atomic \ref int_fast8_t. */
typedef _Atomic __INT_FAST8_TYPE__ atomic_int_fast8_t;

/** Atomic \ref uint_fast8_t. */
typedef _Atomic __UINT_FAST8_TYPE__ atomic_uint_fast8_t;

/** Atomic \ref int_fast16_t. */
typedef _Atomic __INT_FAST16_TYPE__ atomic_int_fast16_t;

/** Atomic \ref uint_fast16_t. */
typedef _Atomic __UINT_FAST16_TYPE__ atomic_uint_fast16_t;

/** Atomic \ref int_fast32_t. */
typedef _Atomic __INT_FAST32_TYPE__ atomic_int_fast32_t;

/** Atomic \ref uint_fast32_t. */
typedef _Atomic __UINT_FAST32_TYPE__ atomic_uint_fast32_t;

/** Atomic \ref int_fast64_t. */
typedef _Atomic __INT_FAST64_TYPE__ atomic_int_fast64_t;

/** Atomic \ref uint_fast64_t. */
typedef _Atomic __UINT_FAST64_TYPE__ atomic_uint_fast64_t;

/** Atomic \ref intptr_t. */
typedef _Atomic __INTPTR_TYPE__ atomic_intptr_t;

/** Atomic \ref uintptr_t. */
typedef _Atomic __UINTPTR_TYPE__ atomic_uintptr_t;

/** Atomic \ref size_t. */
typedef _Atomic __SIZE_TYPE__ atomic_size_t;

/** Atomic \ref ptrdiff_t. */
typedef _Atomic __PTRDIFF_TYPE__ atomic_ptrdiff_t;

/** Atomic \ref intmax_t. */
typedef _Atomic __INTMAX_TYPE__ atomic_intmax_t;

/** Atomic \ref uintmax_t. */
typedef _Atomic __UINTMAX_TYPE__ atomic_uintmax_t;

/**
 * Atomic variable initialization. Can safely be ignored.
 *
 * @param VALUE Value to be used in initialization.
 * @return \c VALUE
 */
#define ATOMIC_VAR_INIT(VALUE) (VALUE)

/**
 * Atomic pointer initialization.
 *
 * @param PTR Pointer to where value should be stored.
 * @param VAL Value to store.
 */
#define atomic_init(PTR, VAL) atomic_store_explicit(PTR, VAL, __ATOMIC_RELAXED)

/**
 * Kill dependency.
 * \todo Figure out what it means.
 *
 * @param y Value whose dependencies should be killed.
 * @return \c y
 */
#define kill_dependency(y) (y)

/**
 * Check if given type is lock free.
 *
 * @param x Type to check.
 * @return \c 0 if never lock free, \c 1 if sometimes lock free, \c 2 if always lock
 * free.
 */
#if defined(__GNUC__)
#define CMPLR_LOCK_FREE(x) GLUE(__GCC_ATOMIC_, x)##_LOCK_FREE
#elif defined(__clang__)
#define CMPLR_LOCK_FREE(x) GLUE(__CLANG_ATOMIC_, x)##_LOCK_FREE
#endif

/** Whether \ref atomic_bool is lock free. \see CMPLR_LOCK_FREE(). */
#define ATOMIC_BOOL_LOCK_FREE CMPLR_LOCK_FREE(BOOL)

/** Whether \ref atomic_char is lock free. \see CMPLR_LOCK_FREE(). */
#define ATOMIC_CHAR_LOCK_FREE CMPLR_LOCK_FREE(CHAR)

/** Whether \ref atomic_char16_t is lock free. \see CMPLR_LOCK_FREE(). */
#define ATOMIC_CHAR16_T_LOCK_FREE CMPLR_LOCK_FREE(CHAR16_T)

/** Whether \ref atomic_char32_t is lock free. \see CMPLR_LOCK_FREE(). */
#define ATOMIC_CHAR32_T_LOCK_FREE CMPLR_LOCK_FREE(CHAR32_T)

/** Whether \ref atomic_wchar_t is lock free. \see CMPLR_LOCK_FREE(). */
#define ATOMIC_WCHAR_T_LOCK_FREE CMPLR_LOCK_FREE(WCHAR32_T)

/** Whether \ref atomic_short is lock free. \see CMPLR_LOCK_FREE(). */
#define ATOMIC_SHORT_LOCK_FREE CMPLR_LOCK_FREE(SHORT)

/** Whether \ref atomic_int is lock free. \see CMPLR_LOCK_FREE(). */
#define ATOMIC_INT_LOCK_FREE CMPLR_LOCK_FREE(INT)

/** Whether \ref atomic_long is lock free. \see CMPLR_LOCK_FREE(). */
#define ATOMIC_LONG_LOCK_FREE CMPLR_LOCK_FREE(LONG)

/** Whether \ref atomic_llong is lock free. \see CMPLR_LOCK_FREE(). */
#define ATOMIC_LLONG_LOCK_FREE CMPLR_LOCK_FREE(LLONG)

/** Whether atomic pointers are lock free. \see CMPLR_LOCK_FREE(). */
#define ATOMIC_POINTER_LOCK_FREE CMPLR_LOCK_FREE(POINTER)

/**
 * Helper macro for creating builtin symbols.
 *
 * @param x Base name of symbol.
 */
#if defined(__GNUC__)
#define C11_ATOMIC(x) GLUE(__atomic_, x)
#elif defined(__clang__)
#define C11_ATOMIC(x) GLUE(__c11_atomic_, x)
#endif

/* no libc, so fences aren't used here (unless implemented, but I don't see that
 * to be necessary)
 *
 * void atomic_thread_fence(memory_order);
 * void atomic_signal_fence(memory_order);
 */

/**
 * Atomic thread fence.
 *
 * @param order Memory ordering. \see memory_order.
 */
#define atomic_thread_fence(order) C11_ATOMIC(thread_fence)(order)

/**
 * Atomic signal fence.
 *
 * @param order Memory ordering. \see memory_order.
 */
#define atomic_signal_fence(order) C11_ATOMIC(signal_fence)(order)

/**
 * Whether object is lock free.
 *
 * @param obj Object to check.
 * @return \ref true if the object is lock free, \ref false otherwise.
 */
#if defined(__GNUC__)
#define atomic_is_lock_free(obj) C11_ATOMIC(is_lock_free)(sizeof(*(obj)), (obj))
#elif defined(__clang__)
#define atomic_is_lock_free(obj) C11_ATOMIC(is_lock_free)(sizeof(*(obj)))
#endif

/**
 * Helper macro for creating some more builtin symbols.
 * GCC uses \c _n suffixes for functions for equivalent functions in Clang, and
 * this macro automatically appends \c _n when needed.
 *
 * @param x Base name of symbol.
 */
#if defined(__GNUC__)
#define N_ATOMIC(x) GLUE(C11_ATOMIC(x), _n)
#elif defined(__clang__)
#define N_ATOMIC(x) C11_ATOMIC(x)
#endif

/**
 * Explicit memory ordering for atomic store.
 *
 * @param obj Pointer to store destination.
 * @param val Value to be stored.
 * @param mode Memory ordering. \see memory_order.
 */
#define atomic_store_explicit(obj, val, mode) N_ATOMIC(store)(obj, val, mode)

/**
 * Atomic memory store with implicit strongest memory ordering.
 *
 * @param obj Pointer to store destination.
 * @param val Value to be stored.
 */
#define atomic_store(obj, val) \
	atomic_store_explicit(obj, val, __ATOMIC_SEQ_CST);

/**
 * Explicit memory ordering for atomic load.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to load source.
 * @param mode Memory ordering. \see memory_order.
 * @return Value at \c obj.
 */
#define atomic_load_explicit(obj, mode) N_ATOMIC(load)(obj, mode)

/**
 * Atomic memory load with implicit strongest memory ordering.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to load source.
 * @return Value at \c obj.
 */
#define atomic_load(obj) \
	atomic_load_explicit(obj, __ATOMIC_SEQ_CST)

/**
 * Explicit memory ordering for atomic exchange.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to exchange destination.
 * @param val Value to be inserted at \c obj.
 * @param mode Memory ordering. \see memory_order.
 * @return Value at \c obj.
 */
#define atomic_exchange_explicit(obj, val, mode) \
	N_ATOMIC(exchange)(obj, val, mode)

/**
 * Atomic memory exchange with implicit strongest memory ordering.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to exchange destination.
 * @param val Value to be inserted at \c obj.
 * @return Value at \c obj.
 */
#define atomic_exchange(obj, val) \
	atomic_exchange_explicit(obj, val, __ATOMIC_SEQ_CST)

/**
 * Explicit strong memory compare and exchange.
 * If \c obj and \c val are equal, the contents of \c obj are replaced with \c
 * des. Otherwise, the contents of \c des is replaced with the contents of \c
 * obj. The exchange is not allowed to spuriously fail.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object to be compared.
 * @param val Value to be compared against.
 * @param des Value to be copied if \c obj and \c val equal.
 * @param suc Memory ordering of succesful exchange. \see memory_order.
 * @param fail Memory ordering of unsuccesful exchange. \see memory_order.
 * @return \ref true if \c obj and \c val equal, \ref false otherwise.
 */
#if defined(__GNUC__)
#define atomic_compare_exchange_strong_explicit(obj, val, des, suc, fail) \
	N_ATOMIC(compare_exchange)(obj, val, des, 0, suc, fail)
#elif defined(__clang__)
#define atomic_compare_exchange_strong_explicit(obj, val, des, suc, fail) \
	N_ATOMIC(compare_exchange_strong)(obj, val, des, suc, fail)
#endif

/**
 * Strong memory compare and exchange with implicit strongest ordering. \see
 * atomic_compare_exchange_strong_explicit().
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object to be compared.
 * @param val Value to be compared agains.
 * @param des Value to be copied if \c obj and \c val equal.
 * @return \ref true if \c obj and \c val equal, \ref false otherwise.
 */
#define atomic_compare_exchange_strong(obj, val, des) \
	atomic_compare_exchange_strong_explicit(      \
		obj, val, des, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

/**
 * Explicit weak memory compare and exchange.
 * If \c obj and \c val are equal, the contents of \c obj are replaced with \c
 * des. Otherwise, the contents of \c des is replaced with the contents of \c
 * obj. The exchange is allowed to spuriously fail.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to be compared against.
 * @param des Value to be copied if \c obj and \c val equal.
 * @param suc Memory ordering on succesful exchange. \see memory_order.
 * @param fail Memory ordering on unsuccesful exchange. \see memory_order.
 * @return \ref true if \c obj and \c val equal and exchange was succesful, \ref
 * false otherwise.
 */
#if defined(__GNUC__)
#define atomic_compare_exchange_weak_explicit(obj, val, des, suc, fail) \
	N_ATOMIC(compare_exchange)(obj, val, des, 1, suc, fail)
#elif defined(__clang__)
#define atomic_compare_exchange_weak_explicit(obj, val, des, suc, fail) \
	N_ATOMIC(compare_exchange_weak)(obj, val, des, suc, fail)
#endif

/**
 * Weak memory compare and exchange with implicit strongest ordering.
 * \see atomic_compare_exchange_weak_explicit().
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to be compared against.
 * @param des Value to be copied if \c obj and \c val equal.
 * @return \ref true if \c obj and \c val equal and exchange was succesful, \ref
 * false otherwise.
 */
#define atomic_compare_exchange_weak(obj, val, des)                            \
	atomic_compare_exchange_weak_explicit(obj, val, des, __ATOMIC_SEQ_CST, \
	                                      __ATOMIC_SEQ_CST)

/**
 * Explicit atomic in-place addition.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to be added to object.
 * @param mode Memory ordering. \see memory_order.
 * @return Contents of \c obj before addition.
 */
#define atomic_fetch_add_explicit(obj, val, mode) \
	C11_ATOMIC(fetch_add)(obj, val, mode)

/**
 * Atomic in-place addition with implicit strongest memory ordering.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to be added to object.
 * @return Contents of \c obj before addition.
 */
#define atomic_fetch_add(obj, val) \
	atomic_fetch_add_explicit(obj, val, __ATOMIC_SEQ_CST)

/**
 * Explicit atomic in-place subtraction.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to be subtracted from object.
 * @param mode Memory ordering. \see memory_order.
 * @return Contents of \c obj before subtraction.
 */
#define atomic_fetch_sub_explicit(obj, val, mode) \
	C11_ATOMIC(fetch_sub)(obj, val, mode)

/**
 * Atomic in-place subtraction with implicit strongest memory ordering.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to be subtracted from object.
 * @return Contents of \c obj before subtraction.
 */
#define atomic_fetch_sub(obj, val) \
	atomic_fetch_sub_explicit(obj, val, __ATOMIC_SEQ_CST)

/**
 * Explicit atomic in-place bitwise \c AND.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to bitwise \c AND with contents of object.
 * @param mode Memory ordering. \see memory_order.
 * @return Contents of \c obj before bitwise \c AND.
 */
#define atomic_fetch_and_explicit(obj, val, mode) \
	C11_ATOMIC(fetch_and)(obj, val, mode)

/**
 * Atomic in-place bitwise \c AND with implicit strongest memory ordering.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to bitwise \c AND with contents of object.
 * @return Contents of \c obj before bitwise \c AND.
 */
#define atomic_fetch_and(obj, val) \
	atomic_fetch_and_explicit(obj, val, __ATOMIC_SEQ_CST)

/**
 * Explicit atomic in-place bitwise \c XOR.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to bitwise \c XOR with contents of object.
 * @param mode Memory ordering. \see memory_order.
 * @return Contents of \c obj before bitwise \c XOR.
 */
#define atomic_fetch_xor_explicit(obj, val, mode) \
	C11_ATOMIC(fetch_xor)(obj, val, mode)

/**
 * Atomic in-place bitwise \c XOR with implicit strongest memory ordering.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to bitwise \c XOR with contents of object.
 * @return Contents of \c obj before bitwise \c XOR.
 */
#define atomic_fetch_xor(obj, val) \
	atomic_fetch_xor_explicit(obj, val, __ATOMIC_SEQ_CST)

/**
 * Explicit atomic in-place bitwise \c OR.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to bitwise \c OR with contents of object.
 * @param mode Memory ordering. \see memory_order.
 * @return Contents of \c obj before bitwise \c OR.
 */
#define atomic_fetch_or_explicit(obj, val, mode) \
	C11_ATOMIC(fetch_or)(obj, val, mode)

/**
 * Atomic in-place bitwise \c OR with implicit strongest memory ordering.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to bitwise \c OR with contents of object.
 * @return Contents of \c obj before bitwise \c OR.
 */
#define atomic_fetch_or(obj, val) \
	atomic_fetch_or_explicit(obj, val, __ATOMIC_SEQ_CST)

/**
 * Explicit atomic in-place bitwise \c NAND.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to bitwise \c OR with contents of object.
 * @param mode Memory ordering. \see memory_order.
 * @return Contents of \c obj before bitwise \c NAND.
 */
#define atomic_fetch_nand_explicit(obj, val, mode) \
	C11_ATOMIC(fetch_nand)(obj, val, mode)

/**
 * Atomic in-place bitwise \c NAND with implicit strongest memory ordering.
 * Type is deduced from \c obj.
 *
 * @param obj Pointer to object.
 * @param val Value to bitwise \c NAND with contents of object.
 * @return Contents of \c obj before bitwise \c NAND.
 */
#define atomic_fetch_nand(obj, val) \
	atomic_fetch_nand_explicit(obj, val, __ATOMIC_SEQ_CST)

/* skip atomic flags, probably not needed */
#endif /* ATOMIC_H */
