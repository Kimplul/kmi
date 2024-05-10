/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023 Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * Header for silencing scripts/warn-undocumented
 *
 * @file init.c
 *
 * Test init program.
 *
 * This file is only temporarily in this tree.
 * At some point in the (hopefully) near future, I intend to move the
 * kernel code and initrd generation stuff into separate repositories, to make
 * things easier for myself. For now though, this is good enough.
 *
 * Compile with
 *	riscv64-unknown-elf-gcc -ffreestanding -nostdlib
 *
 * Create initrd with
 *	echo init | cpio -H newc -o > initrd
 */

#include <stdint.h>
#include <stddef.h>
#include "../../../include/kmi/syscalls.h"

struct sys_ret {
	long a0, a1, a2, a3, a4, a5;
};

char *strcpy(char * restrict dst, const char * restrict src)
{
	const char *s1 = src;
	char *s2 = dst;

	while (*s1)
		*(s2++) = *(s1++);

	return dst;
}

static inline struct sys_ret ecall(size_t n,
                                   long arg0, long arg1, long arg2, long arg3,
                                   long arg4, long arg5)
{
	/* here a static assert of n <= 6 && n >= 1 would be ideal */

	register long a0 asm ("a0") = arg0;
	register long a1 asm ("a1") = arg1;
	register long a2 asm ("a2") = arg2;
	register long a3 asm ("a3") = arg3;
	register long a4 asm ("a4") = arg4;
	register long a5 asm ("a5") = arg5;

#define OUTPUTS "+r" (a0), "=r" (a1), "=r" (a2), "=r" (a3), "=r" (a4), "=r" (a5)

	if (n == 1)
		asm volatile ("ecall" : OUTPUTS : "r" (a0));

	else if (n == 2)
		asm volatile ("ecall" : OUTPUTS : "r" (a0), "r" (a1));

	else if (n == 3)
		asm volatile ("ecall" : OUTPUTS : "r" (a0), "r" (a1), "r" (a2));

	else if (n == 4)
		asm volatile ("ecall" : OUTPUTS
		              : "r" (a0), "r" (a1), "r" (a2), "r" (a3));

	else if (n == 5)
		asm volatile ("ecall" : OUTPUTS
		              : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
		              "r" (a4));

	else if (n == 6)
		asm volatile ("ecall"
		              : OUTPUTS
		              : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
		              "r" (a4), "r" (a5));

#undef OUTPUTS
	return (struct sys_ret){a0, a1, a2, a3, a4, a5};
}

#define ecall1(a) ecall(1, a, 0, 0, 0, 0, 0)
#define ecall2(a, b) ecall(2, a, b, 0, 0, 0, 0)
#define ecall3(a, b, c) ecall(3, a, b, c, 0, 0, 0)
#define ecall4(a, b, c, d) ecall(4, a, b, c, d, 0, 0)
#define ecall5(a, b, c, d, e) ecall(5, a, b, c, d, e, 0)
#define ecall6(a, b, c, d, e, f) ecall(6, a, b, c, d, e, f)

static void sys_noop()
{
	ecall1(SYS_NOOP);
}

static void sys_putch(char c)
{
	ecall2(SYS_PUTCH, c);
}

static uint64_t sys_timebase()
{
	struct sys_ret r = ecall1(SYS_TIMEBASE);
#if defined(_LP64)
	return r.a1;
#else
	uint64_t t = r.a1;
	t <<= 32;
	return t + r.a2;
#endif
}

static uint64_t sys_ticks()
{
	struct sys_ret r = ecall1(SYS_TICKS);
#if defined(_LP64)
	return r.a1;
#else
	uint64_t t = r.a1;
	t <<= 32;
	return t + r.a2;
#endif
}

static void puts(const char *s)
{
	while (*s)
		sys_putch(*s++);
}

static void _print_number(uint64_t v)
{
	if (v == 0)
		return;

	_print_number(v / 10);
	sys_putch((v % 10) + '0');
}

static void print_number(uint64_t v)
{
	if (v == 0) {
		sys_putch('0');
		return;
	}

	_print_number(v);
}

static void print_value(const char *s, uint64_t v)
{
	puts(s);
	puts(": ");

	print_number(v);
	sys_putch('\n');
}

static uint64_t sys_fork()
{
	struct sys_ret r = ecall1(SYS_FORK);

	if (r.a0 != 0) {
		print_value("fork() failed with error ", r.a0);
		return r.a0;
	}

	return r.a1;
}

static uint64_t sys_swap(long tid)
{
	struct sys_ret r = ecall2(SYS_SWAP, tid);

	if (r.a0 != 0) {
		print_value("swap() failed with error ", r.a0);
		return r.a0;
	}

	return 0;
}

static void sys_ipc_server(void *f)
{
	struct sys_ret r = ecall2(SYS_IPC_SERVER, (long)f);

	if (r.a0 != 0)
		print_value("ipc_server() failed with error ", r.a0);
}

/** Helper for ipc arguments/return values. */
struct ipc_args {
	long s; long a0, a1, a2, a3;
};

static inline struct ipc_args sys_ipc_req(long tid, long d0, long d1, long d2,
                                          long d3)
{
	struct sys_ret r = ecall6(SYS_IPC_REQ, tid, d0, d1, d2, d3);

	if (r.a0) {
		print_value("ipc_req() failed with error ", r.a0);
		return (struct ipc_args){r.a0, 0, 0, 0, 0};
	}

	return (struct ipc_args){r.a0, r.a1, r.a2, r.a3, r.a4};
}

static void sys_ipc_resp(long d0, long d1, long d2, long d3)
{
	ecall5(SYS_IPC_RESP, d0, d1, d2, d3);
}

static void sys_poweroff(long type)
{
	ecall2(SYS_POWEROFF, type);
}

static void *sys_req_mem(size_t count)
{
	struct sys_ret r = ecall3(SYS_REQ_MEM, count,
	                          (1 << 0) | (1 << 1) | (1 << 2) | (1 << 4));
	if (r.a0) {
		print_value("sys_req_mem() failed with error ", r.a0);
		return NULL;
	}

	return (void *)r.a1;
}

static void sys_free_mem(void *p)
{
	struct sys_ret r = ecall2(SYS_FREE_MEM, (long)p);

	if (r.a0)
		print_value("sys_free_mem() failed with error ", r.a0);
}

static void *sys_req_sharedmem(long tid, unsigned long size, void **cbuf)
{
	struct sys_ret r = ecall5(SYS_REQ_SHAREDMEM, tid, size,
	                          (1 << 0) | (1 << 1) | (1 << 2) | (1 << 4),
	                          (1 << 0) | (1 << 1) | (1 << 2) | (1 << 4));
	if (r.a0) {
		print_value("sys_req_sharedmem() failed with error ", r.a0);
		return NULL;
	}

	*cbuf = (void *)r.a2;
	return (void *)r.a1;
}

/* I'm guessing my elf parser doesn't handle data pages correctly yet... */
static char *rw_buf = 0;
static size_t rw_buf_size = 4096;

void callback(long status, long tid, long d0, long d1, long d2, long d3)
{
	(void)status;

	if (d0 == 1) {
		void *cbuf = 0;
		rw_buf = sys_req_sharedmem(tid, rw_buf_size, &cbuf);
		sys_ipc_resp((long)cbuf, rw_buf_size, 0, 0);
		__builtin_unreachable();

	} else if (d0 == 2) {
		puts(rw_buf);
		sys_ipc_resp(0, 0, 0, 0);
		__builtin_unreachable();
	}

	sys_ipc_resp(0, 0, 0, 0);
	__builtin_unreachable();
}

#define CSR_TIME "0xc01"
#define csr_read(csr, \
		 res) __asm__ volatile ("csrr %0, " csr : "=r" (res) :: "memory")

#define CSR_CYCLE "0xc00"

void _start()
{
	sys_noop();
	puts("Hello, world!\n");

	uint64_t second = sys_timebase();
	print_value("Timebase", second);

	uint64_t n = 0, i, start, cn, cstart, cend;
	start = i = sys_ticks();
	csr_read(CSR_CYCLE, cstart);
	while (i < start + second) {
		i = sys_ticks();
		n++;
	}
	csr_read(CSR_CYCLE, cend);

	print_value("Start ticks", start);
	print_value("End ticks", i);
	print_value("Syscalls per second", n);
	print_value("Executed cycles per second", cend - cstart);

	puts("Setting callback...");
	sys_ipc_server(callback);

	puts("Starting fork():\n");
	long pid = sys_fork();
	if (pid != 0) {
		print_value("Child pid", pid);
		puts("Swapping to child...\n");
		while(1) sys_swap(pid);
	}

	puts("Hello from child!\n");

	puts("Doing swaps...\n");
	csr_read(CSR_TIME, i);
	start = i; n = 0;
	while (i < start + second) {
		sys_swap(1);
		csr_read(CSR_TIME, i);
		n++;
	}

	print_value("Swaps (both ways) per second", n);

	puts("Doing ipc requests...\n");
	csr_read(CSR_TIME, i);
	start = i; n = 0;
	while (i < start + second) {
		sys_ipc_req(1, 0, 0, 0, 0);
		csr_read(CSR_TIME, i);
		n++;
	}

	print_value("IPC requests per second", n);

	puts("Doing memory allocations...\n");
	for (i = 0; i < 1000000; ++i) {
		char *p = sys_req_mem(10);
		*p = 'c';
		sys_free_mem(p);
	}

	puts("Checking shared memory\n");
	struct ipc_args r = sys_ipc_req(1, 1, 0, 0, 0);
	rw_buf = (char *)r.a1;
	rw_buf_size = (size_t)r.a2;
	print_value("Shared memory ptr", (uintptr_t)rw_buf);
	print_value("Shared memory size", rw_buf_size);

	rw_buf[0] = 0;
	strcpy(rw_buf, "Hello from the other side!\n");
	sys_ipc_req(1, 2, 0, 0, 0);

	sys_poweroff(0);
}
