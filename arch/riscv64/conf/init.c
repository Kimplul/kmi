/* This file is allowed to be undocumented as it is only temporarily in this
 * tree. At some point in the (hopefully) near future, I intend to move the
 * kernel code and initrd generation stuff into separate repositories, to make
 * things easier for myself. For now though, this is good enough.
 */

/* compile with riscv64-unknown-elf-gcc -ffreestanding -nostdlib */
/* create initrd with echo init | cpio -H newc -o > initrd */
#include <stdint.h>
#include "../../../include/apos/syscalls.h"

#define ecall() do { asm volatile ("ecall"                               \
		                   :                                     \
		                   :                                     \
		                   : "a0", "a1", "a2", "a3", "a4", "a5", \
		                   "memory");                            \
} while (0)

static void sys_noop()
{
	long register a0 asm ("a0") = SYS_NOOP;
	ecall();
}

static void sys_putch(char c)
{
	long register a0 asm ("a0") = SYS_PUTCH;
	long register a1 asm ("a1") = c;
	ecall();
}

static uint64_t sys_timebase()
{
	long register a0 asm ("a0") = SYS_TIMEBASE;
	long register a1 asm ("a1") = 0;
	long register a2 asm ("a2") = 0;
	ecall();
#if defined(_LP64)
	return a1;
#else
	uint64_t t = a1;
	t <<= 32;
	return t + a2;
#endif
}

static uint64_t sys_ticks()
{
	long register a0 asm ("a0") = SYS_TICKS;
	long register a1 asm ("a1") = 0;
	long register a2 asm ("a2") = 0;
	ecall();
#if defined(_LP64)
	return a1;
#else
	uint64_t t = a1;
	t <<= 32;
	return t + a2;
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
	long register a0 asm ("a0") = SYS_FORK;
	long register a1 asm ("a1") = 0;
	ecall();
	if (a0 != 0)
		print_value("fork() failed with error ", a0);
	return a1;
}

static uint64_t sys_swap(long tid)
{
	long register a0 asm ("a0") = SYS_SWAP;
	long register a1 asm ("a1") = tid;
	ecall();
	if (a0 != 0)
		print_value("swap() failed with error ", a0);
	return a0;
}

static void sys_ipc_server(void *f)
{
	long register a0 asm ("a0") = SYS_IPC_SERVER;
	long register a1 asm ("a1") = (long)f;
	ecall();
	if (a0 != 0)
		print_value("ipc_server() failed with error ", a0);
}

static uint64_t sys_ipc_req(long tid, long *d0, long *d1, long *d2, long *d3)
{
	long register a0 asm ("a0") = SYS_IPC_REQ;
	long register a1 asm ("a1") = tid;
	long register a2 asm ("a2") = *d0;
	long register a3 asm ("a3") = *d1;
	long register a4 asm ("a4") = *d2;
	long register a5 asm ("a5") = *d3;
	ecall();
	if (a0)
		print_value("ipc_req() failed with error ", a0);

	*d0 = a2;
	*d1 = a3;
	*d2 = a4;
	*d3 = a5;
	return a0;
}

static void sys_ipc_resp(long d0, long d1, long d2, long d3)
{
	long register a0 asm ("a0") = SYS_IPC_RESP;
	long register a1 asm ("a1") = d0;
	long register a2 asm ("a2") = d1;
	long register a3 asm ("a3") = d2;
	long register a4 asm ("a4") = d3;
	ecall();
}

static void sys_poweroff(long type)
{
	long register a0 asm ("a0") = SYS_POWEROFF;
	long register a1 asm ("a1") = type;
	ecall();
}

void callback(long status, long tid, long d0, long d1, long d2, long d3)
{
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
	long d0, d1, d2, d3;
	csr_read(CSR_TIME, i);
	start = i; n = 0;
	while (i < start + second) {
		sys_ipc_req(1, &d0, &d1, &d2, &d3);
		csr_read(CSR_TIME, i);
		n++;
	}

	print_value("IPC requests per second", n);

	sys_poweroff(0);
}
