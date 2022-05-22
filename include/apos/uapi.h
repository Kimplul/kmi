#ifndef APOS_UAPI_H
#define APOS_UAPI_H

/**
 * @file uapi.h
 * Userspace api, syscall declarations.
 */

#include <apos/syscalls.h>
#include <apos/vmem.h>

/* syscall function type, let's start with four arguments and see where that
 * goes */
typedef struct sys_ret (*sys_t)(long, long, long, long);
/* TODO: should this be arch specific? should be the size of an integer register */
typedef long sys_arg_t;

struct sys_ret {
	sys_arg_t err;
	sys_arg_t val;
};

#define SYSCALL_DECLARE(name)                                                  \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c,       \
	                          sys_arg_t d);

#define SYSCALL_DEFINE0(name)                                                  \
	static inline struct sys_ret __##name();                               \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c,       \
	                          sys_arg_t d)                                 \
	{                                                                      \
		UNUSED(a);                                                     \
		UNUSED(b);                                                     \
		UNUSED(c);                                                     \
		UNUSED(d);                                                     \
		return __##name();                                             \
	}                                                                      \
	static struct sys_ret __##name

#define SYSCALL_DEFINE1(name)                                                  \
	static inline struct sys_ret __##name(sys_arg_t);                      \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c,       \
	                          sys_arg_t d)                                 \
	{                                                                      \
		UNUSED(b);                                                     \
		UNUSED(c);                                                     \
		UNUSED(d);                                                     \
		return __##name(a);                                            \
	}                                                                      \
	static inline struct sys_ret __##name

#define SYSCALL_DEFINE2(name)                                                  \
	static inline struct sys_ret __##name(sys_arg_t, sys_arg_t);           \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c,       \
	                          sys_arg_t d)                                 \
	{                                                                      \
		UNUSED(c);                                                     \
		UNUSED(d);                                                     \
		return __##name(a, b);                                         \
	}                                                                      \
	static inline struct sys_ret __##name

#define SYSCALL_DEFINE3(name)                                                  \
	static inline struct sys_ret __##name(sys_arg_t, sys_arg_t,            \
	                                      sys_arg_t);                      \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c,       \
	                          sys_arg_t d)                                 \
	{                                                                      \
		UNUSED(d);                                                     \
		return __##name(a, b, c);                                      \
	}                                                                      \
	static inline struct sys_ret __##name

#define SYSCALL_DEFINE4(name)                                                  \
	static inline struct sys_ret __##name(sys_arg_t, sys_arg_t, sys_arg_t, \
	                                      sys_arg_t);                      \
	struct sys_ret sys_##name(sys_arg_t a, sys_arg_t b, sys_arg_t c,       \
	                          sys_arg_t d)                                 \
	{                                                                      \
		return __##name(a, b, c, d);                                   \
	}                                                                      \
	static inline struct sys_ret __##name

/* noop */
SYSCALL_DECLARE(noop);

/* memory */
SYSCALL_DECLARE(req_mem);
SYSCALL_DECLARE(req_pmem);
SYSCALL_DECLARE(req_fixmem);
SYSCALL_DECLARE(req_sharedmem);
SYSCALL_DECLARE(free_mem);

/* timers */
SYSCALL_DECLARE(timebase);
SYSCALL_DECLARE(req_rel_timer);
SYSCALL_DECLARE(req_abs_timer);
SYSCALL_DECLARE(free_timer);

/* ipc */
SYSCALL_DECLARE(ipc_server);
SYSCALL_DECLARE(ipc_req_proc);
SYSCALL_DECLARE(ipc_req_thread);
SYSCALL_DECLARE(ipc_resp);

/* proc */
SYSCALL_DECLARE(create);
SYSCALL_DECLARE(fork);
SYSCALL_DECLARE(exec);
SYSCALL_DECLARE(signal);
SYSCALL_DECLARE(switch);

/* conf */
SYSCALL_DECLARE(conf);
SYSCALL_DECLARE(poweroff);

struct sys_ret syscall_dispatch(sys_arg_t syscall, sys_arg_t a, sys_arg_t b,
                                sys_arg_t c, sys_arg_t d);

#endif /* APOS_UAPI_H */
