#include <apos/tcb.h>
#include <apos/elf.h>
#include <csr.h>

void jump_to_userspace(struct tcb *t, vm_t bin, int argc, char **argv)
{
	csr_write(CSR_SEPC, prepare_proc(t, bin));
	__asm__("mv sp, %0\n" : "=r" (t->proc_stack) :: "memory");
	__asm__("sret\n" ::: "memory");
}

void return_to_userspace(struct tcb *t)
{
	/* lol */
}
