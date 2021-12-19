#include <apos/tcb.h>
#include <apos/elf.h>
#include <csr.h>

void jump_to_userspace(struct tcb *t, char **argv, int argc)
{
	csr_write(CSR_SEPC, bin_entry(t->bin));
	__asm__("mv sp, %0\n" : "=r" (t->stack) :: "memory");
	__asm__("sret\n" ::: "memory");
}

void return_to_userspace(struct tcb *t)
{
	/* lol */
}
