#ifndef KMI_TESTS_H
#define KMI_TESTS_H

#include <kmi/attrs.h>
#include <kmi/types.h>

#if defined(__riscv)
# if __riscv_xlen == 64
#include "arch/riscv64/syscall.h"
# endif
#endif

#define syscall0(op) syscall(1, op, 0, 0, 0, 0, 0)
#define syscall1(op, a0) syscall(2, op, a0, 0, 0, 0, 0)
#define syscall2(op, a0, a1) syscall(3, op, a0, a1, 0, 0, 0)
#define syscall3(op, a0, a1, a2) syscall(4, op, a0, a1, a2, 0, 0)
#define syscall4(op, a0, a1, a2, a3) syscall(5, op, a0, a1, a2, a3, 0)
#define syscall5(op, a0, a1, a2, a3, a4) syscall(6, op, a0, a1, a2, a3, a4)

#define UNUSED(x) (void)x
#define START(pid, tid, d0, d1, d2, d3)\
void _start(sys_arg_t pid, sys_arg_t tid,\
		sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)

static inline void sys_noop()
{
	syscall0(SYS_NOOP);
}

static inline void sys_putch(char c)
{
	syscall1(SYS_PUTCH, c);
}

static inline void *sys_req_mem(size_t size, vmflags_t flags)
{
	struct sys_ret r = syscall2(SYS_REQ_MEM, size, flags);
	if (r.s)
		return NULL;

	return (void *)r.a0;
}

static inline void *sys_req_fixmem(uintptr_t fixed, size_t size, vmflags_t flags)
{
	struct sys_ret r = syscall3(SYS_REQ_FIXMEM, fixed, size, flags);
	if (r.s)
		return NULL;

	return (void *)r.a0;
}

static inline void *sys_req_pmem(uintptr_t addr, size_t size, vmflags_t flags)
{
	struct sys_ret r = syscall3(SYS_REQ_PMEM, addr, size, flags);
	if (r.s)
		return NULL;

	return (void *)r.a0;
}

static inline void *sys_req_page(size_t size, vmflags_t flags, uintptr_t *addr, size_t *asize)
{
	struct sys_ret r = syscall2(SYS_REQ_PAGE, size, flags);
	if (r.s)
		return NULL;

	if (addr)
		*addr = r.a1;

	if (asize)
		*asize = r.a2;

	return (void *)r.a0;
}

static inline void *sys_req_sharedmem(size_t size, vmflags_t flags)
{
	struct sys_ret r = syscall2(SYS_REQ_SHAREDMEM, size, flags);
	if (r.s)
		return NULL;

	return (void *)r.a0;
}

static inline void *sys_ref_sharedmem(id_t tid, uintptr_t addr, vmflags_t flags)
{
	struct sys_ret r = syscall3(SYS_REF_SHAREDMEM, tid, addr, flags);
	if (r.s)
		return NULL;

	return (void *)r.a0;
}

static inline void sys_free_mem(uintptr_t start)
{
	syscall1(SYS_FREE_MEM, start);
}

static inline uint64_t sys_timebase()
{
	struct sys_ret r = syscall0(SYS_TIMEBASE);
	if (r.s)
		return 0;

#if defined(_LP64)
	return r.a0;
#else
	return ((uint64_t)r.a1 << 32) | r.a0
#endif
}

static inline uint64_t sys_ticks()
{
	struct sys_ret r = syscall0(SYS_TICKS);
	if (r.s)
		return 0;

#if defined(_LP64)
	return r.a0;
#else
	return ((uint64_t)r.a1 << 32) | r.a0;
#endif
}

static inline id_t sys_req_rel_timer(uint64_t ticks)
{
	struct sys_ret r;
#if defined(_LP64)
	r = syscall2(SYS_REQ_REL_TIMER, ticks >> 32, ticks);
#else
	r = syscall1(SYS_REQ_REL_TIMER, ticks);
#endif

	if (r.s)
		return -1;

	return r.a0;
}

static inline id_t sys_req_abs_timer(uint64_t ticks)
{
	struct sys_ret r;
#if defined(_LP64)
	r = syscall2(SYS_REQ_ABS_TIMER, ticks >> 32, ticks);
#else
	r = syscall1(SYS_REQ_ABS_TIMER, ticks);
#endif

	if (r.s)
		return -1;

	return r.a0;
}

#define sys_ipc_req0(pid) syscall1(SYS_IPC_REQ, pid)
#define sys_ipc_req1(pid, d0) syscall2(SYS_IPC_REQ, pid, d0)
#define sys_ipc_req2(pid, d0, d1) syscall3(SYS_IPC_REQ, pid, d0, d1)
#define sys_ipc_req3(pid, d0, d1, d2) syscall4(SYS_IPC_REQ, pid, d0, d1, d2)
#define sys_ipc_req4(pid, d0, d1, d2, d3) syscall5(SYS_IPC_REQ, pid, d0, d1, d2, d3)

#define sys_ipc_fwd0(pid) syscall1(SYS_IPC_FWD, pid)
#define sys_ipc_fwd1(pid, d0) syscall2(SYS_IPC_FWD, pid, d0)
#define sys_ipc_fwd2(pid, d0, d1) syscall3(SYS_IPC_FWD, pid, d0, d1)
#define sys_ipc_fwd3(pid, d0, d1, d2) syscall4(SYS_IPC_FWD, pid, d0, d1, d2)
#define sys_ipc_fwd4(pid, d0, d1, d2, d3) syscall5(SYS_IPC_FWD, pid, d0, d1, d2, d3)

#define sys_ipc_tail0(pid) syscall1(SYS_IPC_TAIL, pid)
#define sys_ipc_tail1(pid, d0) syscall2(SYS_IPC_TAIL, pid, d0)
#define sys_ipc_tail2(pid, d0, d1) syscall3(SYS_IPC_TAIL, pid, d0, d1)
#define sys_ipc_tail3(pid, d0, d1, d2) syscall4(SYS_IPC_TAIL, pid, d0, d1, d2)
#define sys_ipc_tail4(pid, d0, d1, d2, d3) syscall5(SYS_IPC_TAIL, pid, d0, d1, d2, d3)

#define sys_ipc_kick0(pid) syscall1(SYS_IPC_KICK, pid)
#define sys_ipc_kick1(pid, d0) syscall2(SYS_IPC_KICK, pid, d0)
#define sys_ipc_kick2(pid, d0, d1) syscall3(SYS_IPC_KICK, pid, d0, d1)
#define sys_ipc_kick3(pid, d0, d1, d2) syscall4(SYS_IPC_KICK, pid, d0, d1, d2)
#define sys_ipc_kick4(pid, d0, d1, d2, d3) syscall5(SYS_IPC_KICK, pid, d0, d1, d2, d3)

#define sys_ipc_resp0(pid) syscall1(SYS_IPC_RESP, pid)
#define sys_ipc_resp1(pid, d0) syscall2(SYS_IPC_RESP, pid, d0)
#define sys_ipc_resp2(pid, d0, d1) syscall3(SYS_IPC_RESP, pid, d0, d1)
#define sys_ipc_resp3(pid, d0, d1, d2) syscall4(SYS_IPC_RESP, pid, d0, d1, d2)
#define sys_ipc_resp4(pid, d0, d1, d2, d3) syscall5(SYS_IPC_RESP, pid, d0, d1, d2, d3)

#define sys_ipc_ghost0(pid) syscall1(SYS_IPC_GHOST, pid)
#define sys_ipc_ghost1(pid, d0) syscall2(SYS_IPC_GHOST, pid, d0)
#define sys_ipc_ghost2(pid, d0, d1) syscall3(SYS_IPC_GHOST, pid, d0, d1)
#define sys_ipc_ghost3(pid, d0, d1, d2) syscall4(SYS_IPC_GHOST, pid, d0, d1, d2)
#define sys_ipc_ghost4(pid, d0, d1, d2, d3) syscall5(SYS_IPC_GHOST, pid, d0, d1, d2, d3)

static inline enum sys_status sys_set_handler(id_t tid, id_t pid)
{
	struct sys_ret r = syscall2(SYS_SET_HANDLER, tid, pid);
	return r.s;
}

static inline enum sys_status sys_notify(id_t tid)
{
	struct sys_ret r = syscall1(SYS_NOTIFY, tid);
	return r.s;
}

static inline id_t sys_create(uintptr_t func, long d0, long d1, long d2, long d3)
{
	struct sys_ret r = syscall5(SYS_CREATE, func, d0, d1, d2, d3);
	return r.s;
}

/* note that negative IDs are error values */
static inline id_t sys_fork(id_t *new_id)
{
	struct sys_ret r = syscall0(SYS_FORK);
	if (new_id)
		*new_id = r.a0;

	return r.s;
}

static inline enum sys_status sys_exec(uintptr_t bin, uintptr_t interp)
{
	struct sys_ret r = syscall2(SYS_EXEC, bin, interp);
	/* if we reach this point, something's gone wrong */
	return r.s;
}

/* negative IDs mean errors */
static inline id_t sys_spawn(uintptr_t bin, uintptr_t interp)
{
	struct sys_ret r = syscall2(SYS_EXEC, bin, interp);
	return r.s;
}

static inline enum sys_status sys_kill(id_t pid)
{
	struct sys_ret r = syscall1(SYS_KILL, pid);
	return r.s;
}

static inline enum sys_status sys_swap(id_t tid)
{
	struct sys_ret r = syscall1(SYS_SWAP, tid);
	return r.s;
}

static inline enum sys_status sys_conf_set(enum conf_param param, long arg)
{
	struct sys_ret r = syscall2(SYS_SET_CONF, param, arg);
	return r.s;
}

static inline long sys_conf_get(enum conf_param param, long arg)
{
	struct sys_ret r = syscall2(SYS_GET_CONF, param, arg);
	return r.a0;
}

static inline enum sys_status sys_set_cap(id_t tid, enum sys_cap cap)
{
	struct sys_ret r = syscall2(SYS_SET_CAP, tid, cap);
	return r.s;
}

static inline enum sys_status sys_get_cap(id_t tid, enum sys_cap *cap)
{
	struct sys_ret r = syscall1(SYS_GET_CAP, tid);
	*cap = r.a0;
	return r.s;
}

static inline enum sys_status sys_clear_cap(id_t tid, enum sys_cap cap)
{
	struct sys_ret r = syscall2(SYS_CLEAR_CAP, tid, cap);
	return r.s;
}

static inline enum sys_status sys_poweroff(enum poweroff_type type)
{
	struct sys_ret r = syscall1(SYS_POWEROFF, type);
	return r.s;
}

static inline enum sys_status sys_sleep()
{
	struct sys_ret r = syscall0(SYS_SLEEP);
	return r.s;
}

/** note that negative IDs mean errors */
static inline id_t sys_irq_req(long irq)
{
	struct sys_ret r = syscall1(SYS_IRQ_REQ, irq);
	return r.s;
}

static inline enum sys_status sys_free_irq(long irq)
{
	struct sys_ret r = syscall1(SYS_FREE_IRQ, irq);
	return r.s;
}

static inline enum sys_status sys_detach(id_t tid)
{
	struct sys_ret r = syscall1(SYS_DETACH, tid);
	return r.s;
}

static inline enum sys_status sys_exit()
{
	struct sys_ret r = syscall0(SYS_EXIT);
	return r.s;
}

int printf(const char *fmt, ...) __printf;

#define error(x, ...)\
	printf("ERROR: " x # __VA_ARGS__)

#define check(x, y, ...)\
	if (!(x)) {\
		error(y #__VA_ARGS__);\
		sys_poweroff(SYS_SHUTDOWN);\
	}

static inline void ok() {
	printf("OK\n");
	sys_poweroff(SYS_SHUTDOWN);
}

#endif /* KMI_TESTS_H */
