#include <apos/elf.h>
#include <apos/vmem.h>

vm_t bin_entry(vm_t bin)
{
	struct elf_header *e = (struct elf_header *)bin;
	/* TODO: check that address points to a valid ELF executable */
	return ((vm_t)e->e_entry) + bin;
}
