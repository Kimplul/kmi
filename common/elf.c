#include <apos/elf.h>
#include <apos/vmem.h>

vm_t bin_entry(vm_t bin)
{
	struct elf_header *e = (struct elf_header *)bin;
	/* TODO: check that address points to a valid ELF executable */
	/* oh yeah, this only works for DYN objects, fuck */
	/* eh, fuck it, I'll fix this later, today has been a decently
	 * productive day as is */
	return ((vm_t)e->e_entry) + bin;
}
