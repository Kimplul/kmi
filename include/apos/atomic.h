#ifndef ATOMIC_H
#define ATOMIC_H

#include <apos/utils.h> /* GLUE */

typedef enum {
	memory_order_relaxed = __ATOMIC_RELAXED,
	memory_order_consume = __ATOMIC_CONSUME,
	memory_order_acquire = __ATOMIC_ACQUIRE,
	memory_order_release = __ATOMIC_RELEASE,
	memory_order_acq_rel = __ATOMIC_ACQ_REL,
	memory_order_seq_cst = __ATOMIC_SEQ_CST
} memory_order;

typedef _Atomic _Bool atomic_bool;
typedef _Atomic char atomic_char;
typedef _Atomic signed char atomic_schar;
typedef _Atomic unsigned char atomic_uchar;
typedef _Atomic short atomic_short;
typedef _Atomic unsigned short atomic_ushort;
typedef _Atomic int atomic_int;
typedef _Atomic unsigned int atomic_uint;
typedef _Atomic long atomic_long;
typedef _Atomic unsigned long atomic_ulong;
typedef _Atomic long long atomic_llong;
typedef _Atomic unsigned long long atomic_ullong;
typedef _Atomic __CHAR16_TYPE__ atomic_char16_t;
typedef _Atomic __CHAR32_TYPE__ atomic_char32_t;
typedef _Atomic __WCHAR_TYPE__ atomic_wchar_t;
typedef _Atomic __INT_LEAST8_TYPE__ atomic_int_least8_t;
typedef _Atomic __UINT_LEAST8_TYPE__ atomic_uint_least8_t;
typedef _Atomic __INT_LEAST16_TYPE__ atomic_int_least16_t;
typedef _Atomic __UINT_LEAST16_TYPE__ atomic_uint_least16_t;
typedef _Atomic __INT_LEAST32_TYPE__ atomic_int_least32_t;
typedef _Atomic __UINT_LEAST32_TYPE__ atomic_uint_least32_t;
typedef _Atomic __INT_LEAST64_TYPE__ atomic_int_least64_t;
typedef _Atomic __UINT_LEAST64_TYPE__ atomic_uint_least64_t;
typedef _Atomic __INT_FAST8_TYPE__ atomic_int_fast8_t;
typedef _Atomic __UINT_FAST8_TYPE__ atomic_uint_fast8_t;
typedef _Atomic __INT_FAST16_TYPE__ atomic_int_fast16_t;
typedef _Atomic __UINT_FAST16_TYPE__ atomic_uint_fast16_t;
typedef _Atomic __INT_FAST32_TYPE__ atomic_int_fast32_t;
typedef _Atomic __UINT_FAST32_TYPE__ atomic_uint_fast32_t;
typedef _Atomic __INT_FAST64_TYPE__ atomic_int_fast64_t;
typedef _Atomic __UINT_FAST64_TYPE__ atomic_uint_fast64_t;
typedef _Atomic __INTPTR_TYPE__ atomic_intptr_t;
typedef _Atomic __UINTPTR_TYPE__ atomic_uintptr_t;
typedef _Atomic __SIZE_TYPE__ atomic_size_t;
typedef _Atomic __PTRDIFF_TYPE__ atomic_ptrdiff_t;
typedef _Atomic __INTMAX_TYPE__ atomic_intmax_t;
typedef _Atomic __UINTMAX_TYPE__ atomic_uintmax_t;

#define ATOMIC_VAR_INIT(VALUE) (VALUE)
#define atomic_init(PTR, VAL)  atomic_store_explicit(PTR, VAL, __ATOMIC_RELAXED)

#define kill_dependency(y)     (y)

#if defined(__GNUC__)
#define CMPLR_LOCK_FREE(x) GLUE(__GCC_ATOMIC_, x)##_LOCK_FREE
#elif defined(__clang__)
#define CMPLR_LOCK_FREE(x) GLUE(__CLANG_ATOMIC_, x)##_LOCK_FREE
#endif

#define ATOMIC_BOOL_LOCK_FREE     CMPLR_LOCK_FREE(BOOL)
#define ATOMIC_CHAR_LOCK_FREE     CMPLR_LOCK_FREE(CHAR)
#define ATOMIC_CHAR16_T_LOCK_FREE CMPLR_LOCK_FREE(CHAR16_T)
#define ATOMIC_CHAR32_T_LOCK_FREE CMPLR_LOCK_FREE(CHAR32_T)
#define ATOMIC_WCHAR_T_LOCK_FREE  CMPLR_LOCK_FREE(WCHAR32_T)
#define ATOMIC_SHORT_LOCK_FREE    CMPLR_LOCK_FREE(SHORT)
#define ATOMIC_INT_LOCK_FREE      CMPLR_LOCK_FREE(INT)
#define ATOMIC_LONG_LOCK_FREE     CMPLR_LOCK_FREE(LONG)
#define ATOMIC_LLONG_LOCK_FREE    CMPLR_LOCK_FREE(LLONG)
#define ATOMIC_POINTER_LOCK_FREE  CMPLR_LOCK_FREE(POINTER)

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

#define atomic_thread_fence(order) C11_ATOMIC(thread_fence)(order)
#define atomic_signal_fence(order) C11_ATOMIC(signal_fence)(order)

#if defined(__GNUC__)
#define atomic_is_lock_free(obj) C11_ATOMIC(is_lock_free)(sizeof(*(obj)), (obj))
#elif defined(__clang__)
#define atomic_is_lock_free(obj) C11_ATOMIC(is_lock_free)(sizeof(*(obj)))
#endif

#if defined(__GNUC__)
#define N_ATOMIC(x) GLUE(C11_ATOMIC(x), _n)
#elif defined(__clang__)
#define N_ATOMIC(x) C11_ATOMIC(x)
#endif

#define atomic_store_explicit(obj, val, mode) N_ATOMIC(store)(obj, val, mode)

#define atomic_store(obj, val)                                                 \
	atomic_store_explicit(obj, val, __ATOMIC_SEQ_CST);

#define atomic_load_explicit(obj, mode) N_ATOMIC(load)(obj, mode)

#define atomic_load(obj)                atomic_load_explicit(obj, ATOMIC_SEQ_CST)

#define atomic_exchange_explicit(obj, val, mode)                               \
	N_ATOMIC(exchange)(obj, val, mode)

#define atomic_exchange(obj, val)                                              \
	atomic_exchange_explicit(obj, val, __ATOMIC_SEQ_CST)

#if defined(__GNUC__)
#define atomic_compare_exchange_strong_explicit(obj, val, des, suc, fail)      \
	N_ATOMIC(compare_exchange)(obj, val, des, 0, suc, fail)
#elif defined(__clang__)
#define atomic_compare_exchange_strong_explicit(obj, val, des, suc, fail)      \
	N_ATOMIC(compare_exchange_strong)(obj, val, des, suc, fail)
#endif

#define atomic_compare_exchange_strong(obj, val, des)                          \
	atomic_compare_exchange_strong_explicit(                               \
		obj, val, des, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

#if defined(__GNUC__)
#define atomic_compare_exchange_weak_explicit(obj, val, des, suc, fail)        \
	N_ATOMIC(compare_exchange)(obj, val, des, 1, suc, fail)
#elif defined(__clang__)
#define atomic_compare_exchange_weak_explicit(obj, val, des, suc, fail)        \
	N_ATOMIC(compare_exchange_weak)(obj, val, des, suc, fail)
#endif

#define atomic_compare_exchange_weak(obj, val, des)                            \
	atomic_compare_exchange_weak_explicit(obj, val, des, __ATOMIC_SEQ_CST, \
	                                      __ATOMIC_SEQ_CST)

#define atomic_fetch_add_explicit(obj, val, mode)                              \
	C11_ATOMIC(fetch_add)(obj, val, mode)

#define atomic_fetch_add(obj, val)                                             \
	atomic_fetch_add_explicit(obj, val, __ATOMIC_SEQ_CST)

#define atomic_fetch_sub_explicit(obj, val, mode)                              \
	C11_ATOMIC(fetch_sub)(obj, val, mode)

#define atomic_fetch_sub(obj, val)                                             \
	atomic_fetch_sub_explicit(obj, val, __ATOMIC_SEQ_CST)

#define atomic_fetch_and_explicit(obj, val, mode)                              \
	C11_ATOMIC(fetch_and)(obj, val, mode)

#define atomic_fetch_and(obj, val)                                             \
	atomic_fetch_and_explicit(obj, val, __ATOMIC_SEQ_CST)

#define atomic_fetch_xor_explicit(obj, val, mode)                              \
	C11_ATOMIC(fetch_xor)(obj, val, mode)

#define atomic_fetch_xor(obj, val)                                             \
	atomic_fetch_xor_explicit(obj, val, __ATOMIC_SEQ_CST)

#define atomic_fetch_or_explicit(obj, val, mode)                               \
	C11_ATOMIC(fetch_or)(obj, val, mode)

#define atomic_fetch_or(obj, val)                                              \
	atomic_fetch_or_explicit(obj, val, __ATOMIC_SEQ_CST)

#define atomic_fetch_nand_explicit(obj, val, mode)                             \
	C11_ATOMIC(fetch_nand)(obj, val, mode)

#define atomic_fetch_nand(obj, val)                                            \
	atomic_fetch_nand_explicit(obj, val, __ATOMIC_SEQ_CST)

/* skip atomic flags, probably not needed */
#endif /* ATOMIC_H */
