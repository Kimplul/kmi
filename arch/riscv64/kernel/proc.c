#include <apos/tcb.h>
#include <apos/elf.h>
#include <csr.h>

void jump_to_userspace(struct tcb *t, int argc, char **argv)
{
	csr_write(CSR_SEPC, t->entry);
	__asm__ volatile ("mv sp, %0\n" : "=r" (t->proc_stack) :: "memory");
	__asm__ volatile ("sret\n" ::: "memory");
}

void return_to_userspace(struct tcb *t)
{
	/* lol */
}
