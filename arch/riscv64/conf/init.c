/* This file is allowed to be undocumented as it is only temporarily in this
 * tree. At some point in the (hopefully) near future, I intend to move the
 * kernel code and initrd generation stuff into separate repositories, to make
 * things easier for myself. For now though, this is good enough.
 */

/* compile with riscv64-unknown-elf-gcc -ffreestanding -nostdlib */
/* create initrd with echo init | cpio -H newc -o > initrd */
#include <stdint.h>
#include "../../../include/apos/syscalls.h"

#define ecall() do { asm ("ecall" : : : "a0", "a1", "a2", "a3", "a4", "a5"); \
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
}
