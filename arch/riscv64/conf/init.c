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
 *	riscv64-unknown-elf-gcc -Driscv64 -ffreestanding -nostdlib
 *
 * Create initrd with
 *	echo init | cpio -H newc -o > initrd
 */

#include <stdint.h>
#include <stddef.h>
#include "../../../include/kmi/syscalls.h"

char *strcpy(char * restrict dst, const char * restrict src)
{
	const char *s1 = src;
	char *s2 = dst;

	while (*s1)
		*(s2++) = *(s1++);

	return dst;
}

static inline struct sys_ret ecall(size_t n,
                                   sys_arg_t arg0, sys_arg_t arg1,
                                   sys_arg_t arg2, sys_arg_t arg3,
                                   sys_arg_t arg4, sys_arg_t arg5)
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
	return r.ar0;
#else
	uint64_t t = r.ar0;
	t <<= 32;
	return t + r.ar1;
#endif
}

static uint64_t sys_ticks()
{
	struct sys_ret r = ecall1(SYS_TICKS);
#if defined(_LP64)
	return r.ar0;
#else
	uint64_t t = r.ar0;
	t <<= 32;
	return t + r.ar1;
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

	if (r.s != 0) {
		print_value("fork() failed with error ", r.s);
		return r.s;
	}

	return r.ar0;
}

static uint64_t sys_swap(long tid)
{
	struct sys_ret r = ecall2(SYS_SWAP, tid);

	if (r.s != 0) {
		print_value("swap() failed with error ", r.s);
		return r.s;
	}

	return 0;
}

static void sys_ipc_server(void *f)
{
	struct sys_ret r = ecall2(SYS_IPC_SERVER, (long)f);

	if (r.s != 0)
		print_value("ipc_server() failed with error ", r.s);
}

static inline struct sys_ret sys_ipc_req(sys_arg_t pid,
                                         sys_arg_t d0, sys_arg_t d1,
                                         sys_arg_t d2,
                                         sys_arg_t d3)
{
	struct sys_ret r = ecall6(SYS_IPC_REQ, pid, d0, d1, d2, d3);

	if (r.s)
		print_value("ipc_req() failed with error ", r.s);

	return r;
}

static void sys_ipc_resp(sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)
{
	ecall5(SYS_IPC_RESP, d0, d1, d2, d3);
}

static void sys_poweroff(long type)
{
	ecall2(SYS_POWEROFF, type);
}

static void *sys_req_mem(size_t count)
{
	struct sys_ret r = ecall3(SYS_REQ_MEM, count, VM_R | VM_W);
	if (r.s) {
		print_value("sys_req_mem() failed with error ", r.s);
		return NULL;
	}

	return (void *)r.ar0;
}

static size_t get_ram_usage()
{
	struct sys_ret r = ecall2(SYS_CONF_GET, CONF_RAM_USAGE);
	if (r.s) {
		print_value("getting RAM usage failed with error ", r.s);
		return 0;
	}

	return r.ar0;
}

static size_t get_ram_size()
{
	struct sys_ret r = ecall2(SYS_CONF_GET, CONF_RAM_SIZE);
	if (r.s) {
		print_value("getting RAM size failed with error ", r.s);
		return 0;
	}

	return r.ar0;
}

static void sys_free_mem(void *p)
{
	struct sys_ret r = ecall2(SYS_FREE_MEM, (long)p);

	if (r.s)
		print_value("sys_free_mem() failed with error ", r.s);
}

static void *sys_req_sharedmem(long tid, unsigned long size, void **cbuf)
{
	struct sys_ret r = ecall3(SYS_REQ_SHAREDMEM, size, VM_R | VM_W);
	if (r.s) {
		print_value("sys_req_sharedmem() failed with error ", r.s);
		return NULL;
	}

	void *rw_buf = (void *)r.ar0;

	r = ecall4(SYS_REF_SHAREDMEM, tid, (sys_arg_t)rw_buf, VM_R | VM_W);
	if (r.s) {
		print_value("sys_ref_sharedmem() failed with error ", r.s);
		return NULL;
	}

	*cbuf = (void *)r.ar0;
	return rw_buf;
}

/* I'm guessing my elf parser doesn't handle data pages correctly yet... */
static char *rw_buf = 0;
static size_t rw_buf_size = 4096;

static void sys_sleep()
{
	ecall1(SYS_SLEEP);
}

#define CSR_TIME "0xc01"
#define csr_read(csr, \
		 res) __asm__ volatile ("csrr %0, " csr : "=r" (res) :: "memory")

#define CSR_CYCLE "0xc00"

static void handle_kernel(long a0, long a1, long d0, long d1, long d2, long d3)
{
	if (a1 != SYS_USER_BOOTED)
		return;

	long tid = d0;
	if (tid != 1) {
		puts("Woo, more cores!\n");
		while (1)
			sys_sleep();
	}

	/* otherwise */
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

	size_t ram = get_ram_usage();
	size_t size = get_ram_size();
	print_value("Memory usage before allocations: ", ram);
	print_value("Memory size in total: ", size);
	for (i = 0; i < 1000000; ++i) {
		char *p = sys_req_mem(10);
		*p = 'c';
		sys_free_mem(p);
	}

	ram = get_ram_usage();
	print_value("Memory usage after allocations: ", ram);

	puts("Checking shared memory\n");
	struct sys_ret r = sys_ipc_req(1, 1, 0, 0, 0);
	rw_buf = (char *)r.ar1;
	rw_buf_size = (size_t)r.ar2;
	print_value("Response from", r.ar0);
	print_value("Shared memory ptr", (uintptr_t)rw_buf);
	print_value("Shared memory size", rw_buf_size);

	rw_buf[0] = 0;
	puts("Sending string...\n");
	strcpy(rw_buf, "Hello from the other side!\n");
	sys_ipc_req(1, 2, 0, 0, 0);

	sys_poweroff(0);
}

void _start(long a0, long a1, long d0, long d1, long d2, long d3)
{
	if (a0 == 0)
		handle_kernel(a0, a1, d0, d1, d2, d3);

	/* otherwise, implement test functionality */
	long pid = a0;
	long tid = a1;
	if (d0 == 1) {
		void *cbuf = 0;
		rw_buf = sys_req_sharedmem(pid, rw_buf_size, &cbuf);
		sys_ipc_resp((long)cbuf, rw_buf_size, 0, 0);
		__builtin_unreachable();

	} else if (d0 == 2) {
		puts("Received string: ");
		puts(rw_buf);
		sys_ipc_resp(0, 0, 0, 0);
		__builtin_unreachable();
	}

	sys_ipc_resp(0, 0, 0, 0);
	__builtin_unreachable();
}
