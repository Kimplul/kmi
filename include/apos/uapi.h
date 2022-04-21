#ifndef APOS_UAPI_H
#define APOS_UAPI_H

#include <apos/syscalls.h>
#include <apos/vmem.h>

/* syscall function type, let's start with four arguments and see where that
 * goes */
typedef vm_t (*sys_t)(vm_t, vm_t, vm_t, vm_t);

#define SYSCALL_DECLARE(name)\
	vm_t sys_##name(vm_t a, vm_t b, vm_t c, vm_t d);

#define SYSCALL_DEFINE0(name)\
	static vm_t __##name();\
	vm_t sys_##name(vm_t a, vm_t b, vm_t c, vm_t d){ \
		UNUSED(a);\
		UNUSED(b);\
		UNUSED(c);\
		UNUSED(d);\
		return __##name();\
	}\
	static vm_t __##name

#define SYSCALL_DEFINE1(name)\
	static vm_t __##name(vm_t);\
	vm_t sys_##name(vm_t a, vm_t b, vm_t c, vm_t d){ \
		UNUSED(b);\
		UNUSED(c);\
		UNUSED(d);\
		return __##name(a);\
	}\
	static vm_t __##name

#define SYSCALL_DEFINE2(name)\
	static vm_t __##name(vm_t, vm_t);\
	vm_t sys_##name(vm_t a, vm_t b, vm_t c, vm_t d){ \
		UNUSED(c);\
		UNUSED(d);\
		return __##name(a, b);\
	}\
	static vm_t __##name

#define SYSCALL_DEFINE3(name)\
	static vm_t __##name(vm_t, vm_t, vm_t);\
	vm_t sys_##name(vm_t a, vm_t b, vm_t c, vm_t d){ \
		UNUSED(d);\
		return __##name(a, b, c);\
	}\
	static vm_t __##name

#define SYSCALL_DEFINE4(name)\
	static vm_t __##name(vm_t, vm_t, vm_t, vm_t);\
	vm_t sys_##name(vm_t a, vm_t b, vm_t c, vm_t d){ \
		return __##name(a, b, c, d);\
	}\
	static vm_t __##name

/* memory */
SYSCALL_DECLARE(req_mem);
SYSCALL_DECLARE(req_pmem);
SYSCALL_DECLARE(req_fixmem);
SYSCALL_DECLARE(req_sharedmem);
SYSCALL_DECLARE(free_mem);

/* timers */
SYSCALL_DECLARE(req_rel_timer);
SYSCALL_DECLARE(req_abs_timer);
SYSCALL_DECLARE(free_timer);

/* ipc */
SYSCALL_DECLARE(ipc_server);
SYSCALL_DECLARE(ipc_req);
SYSCALL_DECLARE(ipc_resp);

/* proc */
SYSCALL_DECLARE(fork);
SYSCALL_DECLARE(exec);
SYSCALL_DECLARE(signal);
SYSCALL_DECLARE(switch);
SYSCALL_DECLARE(sync);

/* conf */
SYSCALL_DECLARE(conf);
SYSCALL_DECLARE(poweroff);

vm_t syscall_dispatch(vm_t syscall, vm_t a, vm_t b, vm_t c, vm_t d);

#endif /* APOS_UAPI_H */
