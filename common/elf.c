/**
 * @file elf.c
 * Handle elf executables, set up requested memory mappings etc.
 */

#include <apos/elf.h>
#include <apos/vmem.h>
#include <apos/bits.h>
#include <apos/string.h>
#include <apos/assert.h>

static uint8_t __elf_to_uvflags(uint8_t elf_flags)
{
	uint8_t uvflags = VM_V | VM_U;
	if (elf_flags & PF_X)
		uvflags |= VM_X;

	if (elf_flags & PF_W)
		uvflags |= VM_W;

	if (elf_flags & PF_R)
		uvflags |= VM_R;

	return uvflags;
}

/* useful bit of info: all segments are sorted in ascending order of p_vaddr */
static void __map_exec(struct tcb *t, vm_t bin, uint8_t ei_c, vm_t phstart,
                       size_t phnum, size_t phsize)
{
	hard_assert(t && is_proc(t), RETURN_VOID);

	/** \todo take alignment into consideration? */
	/** \todo take overlapping memory regions into account, probably mostly
	 * by keeping track of previously allocated area and seeing if the
	 * segment fits into it */
	/** \todo check if p_memsz is larger than p_filesz, the segment should be
	 * filled with zeroes. */
	/** \todo in general, make this a low more clean. */
	vm_t runner = phstart;
	vmflags_t default_flags = VM_V | VM_R | VM_W | VM_X | VM_U;
	for (size_t i = 0; i < phnum; ++i, runner += phsize) {
		if (program_header_prop(ei_c, runner, p_type) != PT_LOAD)
			continue;

		vm_t va = program_header_prop(ei_c, runner, p_vaddr);
		size_t vsz = program_header_prop(ei_c, runner, p_memsz);

		vm_t start = alloc_fixed_region(&t->sp_r, va, vsz, &vsz,
		                                default_flags);
		if (!start)
			return; /* out of memory or something */

		uint8_t elf_flags = program_header_prop(ei_c, runner, p_flags);
		uint8_t uvflags = __elf_to_uvflags(elf_flags);

		map_allocd_region(t->proc.vmem, start, vsz, default_flags, 0);

		vm_t vo = bin + program_header_prop(ei_c, runner, p_offset);
		vm_t vfz = program_header_prop(ei_c, runner, p_filesz);
		memcpy((void *)va, (void *)vo, vfz);

		/* skip while testing
		 * \todo: also fix, this modifies only the first region. Create new
		 * function?
		 *
		   pm_t paddr = 0;
		   stat_vpage(t->b_r, va, &paddr, 0, 0);
		   mod_vpage(t->b_r, va, paddr, uvflags);
		 */
	}
}

static vm_t __map_dyn(struct tcb *t, vm_t bin, uint8_t ei_c, vm_t phstart,
                      size_t phnum, size_t phsize)
{
	/** \todo this path should only be taken when no PT_INTERP is defined, as
	 * making sure ld is loaded should be done in userspace. Maybe a bit
	 * hacky, I know.*/
}

static vm_t __prepare_proc(struct tcb *t, uint8_t ei_c, vm_t elf, vm_t interp)
{
	short e_type = elf_header_prop(ei_c, elf, e_type);
	if (e_type != ET_DYN && e_type != ET_EXEC)
		return 0;

	vm_t phstart = ptradd(elf, elf_header_prop(ei_c, elf, e_phoff));
	size_t phnum = elf_header_prop(ei_c, elf, e_phnum);
	size_t phsize = elf_header_prop(ei_c, elf, e_phentsize);

	vm_t entry = elf_header_prop(ei_c, elf, e_entry);
	if (e_type == ET_EXEC) {
		__map_exec(t, elf, ei_c, phstart, phnum, phsize);
		return entry;
	} else {
		vm_t o = __map_dyn(t, elf, ei_c, phstart, phnum, phsize);
		return o + entry;
	}
}

/* sets up all memory regions etc, returns the entry address */
vm_t load_elf(struct tcb *t, vm_t elf, vm_t interp)
{
	struct elf_ident *i = (struct elf_ident *)elf;
	if (i->ei_magic != cpu_to_be32(EI_MAGIC))
		return 0;

	if (i->ei_class != ELFCLASS32 && i->ei_class != ELFCLASS64)
		return 0;

	/* more sanity checks? */
	return __prepare_proc(t, i->ei_class, elf, interp);
}
