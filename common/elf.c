#include <apos/elf.h>
#include <apos/vmem.h>
#include <apos/bytes.h>
#include <apos/string.h>

static void __map_exec(struct tcb *t, vm_t bin, uint8_t ei_c, vm_t phstart, size_t phnum, size_t phsize)
{
	/* TODO: take alignment into consideration? */
	vm_t runner = phstart;
	for(size_t i = 0; i < phnum; ++i, runner += phsize){
		if(program_header_prop(ei_c, runner, p_type) != PT_LOAD)
			continue;

		vm_t va = program_header_prop(ei_c, runner, p_vaddr);
		size_t vsz = program_header_prop(ei_c, runner, p_memsz);

		vm_t start = 0;
		if(!(start = alloc_fixed_region(&t->sp_r, va, vsz, &vsz)))
				return; /* out of memory or something */

		uint8_t vflags = VM_V | VM_U;
		uint8_t bflags = program_header_prop(ei_c, runner, p_flags);
		if(bflags & PF_X)
			vflags |= VM_X;

		if(bflags & PF_W)
			vflags |= VM_W;

		if(bflags & PF_R)
			vflags |= VM_R;

		map_fill_region(t->b_r, start, vsz, VM_V | VM_X | VM_R | VM_W | VM_U);

		vm_t vo = bin + program_header_prop(ei_c, runner, p_offset);
		vm_t vfz = program_header_prop(ei_c, runner, p_filesz);
		memcpy((void *)va, (void *)vo, vfz);

		/* skip while testing
		 * TODO: also fix, this fixes only the first region. Create new
		 * function?
		 *
		pm_t paddr = 0;
		stat_vmem(t->b_r, va, &paddr, 0, 0);
		mod_vmem(t->b_r, va, paddr, vflags);
		*/
	}
}

static vm_t __map_dyn(struct tcb *t, vm_t bin, uint8_t ei_c, vm_t phstart, size_t phnum, size_t phsize)
{
	/* TODO */
}

static vm_t __prepare_proc(struct tcb *t, uint8_t ei_c, vm_t elf)
{
	short e_type = elf_header_prop(ei_c, elf, e_type);
	if(e_type != ET_DYN && e_type != ET_EXEC)
		return 0;

	vm_t phstart = ptradd(elf, elf_header_prop(ei_c, elf, e_phoff));
	size_t phnum = elf_header_prop(ei_c, elf, e_phnum);
	size_t phsize = elf_header_prop(ei_c, elf, e_phentsize);

	vm_t entry = elf_header_prop(ei_c, elf, e_entry);
	if(e_type == ET_EXEC){
		__map_exec(t, elf, ei_c, phstart, phnum, phsize);
		return entry;
	} else {
		vm_t o = __map_dyn(t, elf, ei_c, phstart, phnum, phsize);
		return o + entry;
	}
}

/* sets up all memory regions etc, returns the entry address */
vm_t prepare_proc(struct tcb *t, vm_t p)
{
	struct elf_ident *i = (struct elf_ident *)p;
	if(i->ei_magic != cpu_to_be32(EI_MAGIC))
		return 0;

	if(i->ei_class != ELFCLASS32 && i->ei_class != ELFCLASS64)
			return 0;

	/* more sanity checks? */
	return __prepare_proc(t, i->ei_class, p);
}
