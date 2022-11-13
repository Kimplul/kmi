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
#include "../../../include/apos/syscalls.h"

struct sys_ret {
	long a0, a1, a2, a3, a4, a5;
};

struct sys_ret ecall(struct sys_ret s)
{
	register long a0 asm ("a0") = s.a0;
	register long a1 asm ("a1") = s.a1;
	register long a2 asm ("a2") = s.a2;
	register long a3 asm ("a3") = s.a3;
	register long a4 asm ("a4") = s.a4;
	register long a5 asm ("a5") = s.a5;

	asm volatile ("ecall"
	              : "=r" (a0), "=r" (a1), "=r" (a2), "=r" (a3), "=r" (a4),
	              "=r" (a5)
	              : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
	              "r" (a5));

	return (struct sys_ret){a0, a1, a2, a3, a4, a5};
}

static void sys_noop()
{
	struct sys_ret r = {.a0 = SYS_NOOP};
	ecall(r);
}

static void sys_putch(char c)
{
	struct sys_ret r = {.a0 = SYS_PUTCH, .a1 = c};
	ecall(r);
}

static uint64_t sys_timebase()
{
	struct sys_ret r = {.a0 = SYS_TIMEBASE};
	r = ecall(r);
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
	struct sys_ret r = {.a0 = SYS_TICKS};
	r = ecall(r);
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
	struct sys_ret r = {.a0 = SYS_FORK};
	r = ecall(r);

	if (r.a0 != 0)
		print_value("fork() failed with error ", r.a0);

	return r.a1;
}

static uint64_t sys_swap(long tid)
{
	struct sys_ret r = {.a0 = SYS_SWAP, .a1 = tid};
	r = ecall(r);

	if (r.a0 != 0)
		print_value("swap() failed with error ", r.a0);

	return r.a0;
}

static void sys_ipc_server(void *f)
{
	struct sys_ret r = {.a0 = SYS_IPC_SERVER, .a1 = (long)f};
	r = ecall(r);

	if (r.a0 != 0)
		print_value("ipc_server() failed with error ", r.a0);
}

/** Helper for ipc arguments/return values. */
struct ipc_args {
	long a0, a1, a2, a3;
};

static struct ipc_args sys_ipc_req(long tid, long d0, long d1, long d2, long d3)
{
	struct sys_ret r = {.a0 = SYS_IPC_REQ,
		            .a1 = tid,
		            .a2 = d0,
		            .a3 = d1,
		            .a4 = d2,
		            .a5 = d3};

	r = ecall(r);

	if (r.a0)
		print_value("ipc_req() failed with error ", r.a0);

	return (struct ipc_args){r.a2, r.a3, r.a4, r.a5};
}

static void sys_ipc_resp(long d0, long d1, long d2, long d3)
{
	struct sys_ret r = {.a0 = SYS_IPC_RESP,
		            .a1 = d0,
		            .a2 = d1,
		            .a3 = d2,
		            .a4 = d3};

	ecall(r);
}

static void sys_poweroff(long type)
{
	struct sys_ret r = {.a0 = SYS_POWEROFF, .a1 = type};
	ecall(r);
}

void callback(long status, long tid, long d0, long d1, long d2, long d3)
{
	(void)status;
	(void)tid;
	sys_ipc_resp(d0, d1, d2, d3);
}

#define CSR_TIME "0xc01"
#define csr_read(csr, \
	         res) __asm__ volatile ("csrr %0, " csr : "=r" (res) :: "memory")

void _start()
{
	sys_noop();
	puts("Hello, world!\n");

	uint64_t second = sys_timebase();
	print_value("Timebase", second);

	uint64_t n = 0, i, start;
	start = i = sys_ticks();
	print_value("Start ticks", start);

	while (i < start + second) {
		i = sys_ticks();
		n++;
	}

	print_value("End ticks", i);
	print_value("Syscalls per second", n);

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
	long d0 = 0, d1 = 0, d2 = 0, d3 = 0;
	csr_read(CSR_TIME, i);
	start = i; n = 0;
	while (i < start + second) {
		sys_ipc_req(1, d0, d1, d2, d3);
		csr_read(CSR_TIME, i);
		n++;
	}

	print_value("IPC requests per second", n);

	sys_poweroff(0);
}
