/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file elf.c
 * Handle elf executables, set up requested memory mappings etc.
 */

#include <kmi/elf.h>
#include <kmi/vmem.h>
#include <kmi/bits.h>
#include <kmi/string.h>
#include <kmi/assert.h>

/**
 * Convert ELF flags to page flags.
 *
 * @param elf_flags ELF flags to convert.
 * @return Corresponding page flags.
 */
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

/**
 * Helper for actually copying ELF data into memory.
 *
 * @param t Process whose virtual memory to work in.
 * @param va Virtual address to where we should write things.
 * @param vaz Size of section in virtual memory.
 * @param vf Address of ELF section from where to read things.
 * @param vfz Size of section in file.
 * @param flags Flags to use for section mapping.
 * @return OK.
 */
static stat_t __elf_map_section(struct tcb *t,
                                vm_t va, size_t vaz,
                                vm_t vf, size_t vfz,
                                uint8_t flags)
{
	size_t region_size;
	vm_t v = alloc_fixed_region(&t->uvmem.region, va, vaz, &region_size,
	                            flags);
	if (!v)
		return ERR_INVAL;

	assert(v == align_down(va, BASE_PAGE_SIZE));

	for (size_t runner = 0; runner < vaz; runner += BASE_PAGE_SIZE) {
		pm_t page = alloc_page(BASE_PAGE);
		if (!page)
			return ERR_OOMEM;

		/* always zero out pages */
		memset((void *)page, 0, BASE_PAGE_SIZE);

		/* sometimes fill page with actual data */
		if (runner < vfz) {
			size_t z = MIN(BASE_PAGE_SIZE, vfz - runner);
			memcpy((void *)page, (void *)(vf + runner), z);
		}

		if (map_vpage(t->proc.vmem,
		              page, va + runner,
		              flags, BASE_PAGE)) {
			free_page(BASE_PAGE, page);
			return ERR_OOMEM;
		}
	}

	return OK;
}

/**
 * Map ELF executable.
 *
 * @param t Thread space to work in.
 * @param bin Address of binary to map.
 * @param ei_c ELF identity class.
 * @param phstart Program header start.
 * @param phnum Number of program header entries.
 * @param phsize Size of page header entry.
 * @return OK on success, something else otherwise.
 */
static stat_t __map_exec(struct tcb *t,
                         vm_t bin,
                         uint8_t ei_c,
                         vm_t phstart,
                         size_t phnum,
                         size_t phsize)
{
	assert(t && is_proc(t));

	/* create empty vmem to work in, allows us to build a new 'copy'
	 * with the required ELF sections mapped. On success, we can just
	 * swap out the wanted vmem for the new one, and on error, just
	 * delete the working vmem. */
	struct vmem *new_vmem = create_vmem();
	if (!new_vmem)
		return ERR_OOMEM;

	struct vmem *old_vmem = t->proc.vmem;
	t->proc.vmem = new_vmem;

	/* create new uvmem for same reason */
	struct uvmem old_uvmem = t->uvmem;
	t->uvmem = (struct uvmem){0};

	if (init_uvmem(t)) {
		destroy_vmem(new_vmem);
		t->uvmem = old_uvmem;
		t->proc.vmem = old_vmem;
		return ERR_OOMEM;
	}

	/** \todo take alignment into consideration? */
	/* useful bit of info: all segments are sorted in ascending order of p_vaddr */
	vm_t runner = phstart;
	for (size_t i = 0; i < phnum; ++i, runner += phsize) {
		if (program_header_prop(ei_c, runner, p_type) != PT_LOAD)
			continue;

		/* where to map section in virtual memory */
		vm_t va = program_header_prop(ei_c, runner, p_vaddr);
		size_t vsz = program_header_prop(ei_c, runner, p_memsz);

		/* where section is in binary */
		vm_t vf = bin + program_header_prop(ei_c, runner, p_offset);
		vm_t vfz = program_header_prop(ei_c, runner, p_filesz);

		uint8_t elf_flags = program_header_prop(ei_c, runner, p_flags);
		uint8_t uvflags = __elf_to_uvflags(elf_flags);

		if (__elf_map_section(t, va, vsz, vf, vfz, uvflags)) {
			destroy_uvmem(t);

			t->proc.vmem = old_vmem;
			t->uvmem = old_uvmem;
			return ERR_OOMEM;
		}
	}

	/* destroy old uvmem (kind of annoying to have it so agressively tied to
	 * the tcb but I guess it's find for now) */
	struct uvmem new_uvmem = t->uvmem;
	t->uvmem = old_uvmem;
	destroy_uvmem(t);

	t->proc.vmem = new_vmem;
	t->uvmem = new_uvmem;
	return OK;
}

/**
 * Map static binary.
 *
 * @param t Process to map binary in.
 * @param ei_c Elf class.
 * @param elf Address of static binary.
 * @return Entry address or NULL.
 */
static vm_t __prepare_exec(struct tcb *t, uint8_t ei_c, vm_t elf)
{
	vm_t phstart = ptradd(elf, elf_header_prop(ei_c, elf, e_phoff));
	size_t phnum = elf_header_prop(ei_c, elf, e_phnum);
	size_t phsize = elf_header_prop(ei_c, elf, e_phentsize);

	vm_t entry = elf_header_prop(ei_c, elf, e_entry);
	if (__map_exec(t, elf, ei_c, phstart, phnum, phsize))
		return 0;

	return entry;
}

/**
 * Map binary and optional interpreter.
 *
 * \todo Implement interpeter handling.
 *
 * @param t Thread space to work in.
 * @param ei_c ELF identity class. (Of binary, should do one for interp?)
 * @param elf ELF binary.
 * @param interp ELF interpeter.
 * @return Entry address.
 */
static vm_t __prepare_proc(struct tcb *t, uint8_t ei_c, vm_t elf, vm_t interp)
{
	UNUSED(interp);

	short e_type = elf_header_prop(ei_c, elf, e_type);

	if (e_type == ET_EXEC)
		return __prepare_exec(t, ei_c, elf);

	if (e_type == ET_DYN) {
		bug("dynamic elf not currently implemented\n");
		return 0;
	}

	return 0;
}

/* sets up all memory regions etc, returns the entry address */
vm_t load_elf(struct tcb *t, vm_t elf, vm_t interp)
{
	/** @todo check that all of elf is in memory, theoretically we could be
	 * passed like half an elf object? */
	struct elf_ident *i = (struct elf_ident *)elf;
	if (i->ei_magic != cpu_to_be32(EI_MAGIC))
		return 0;

	if (i->ei_class != ELFCLASS32 && i->ei_class != ELFCLASS64)
		return 0;

	/* more sanity checks? */
	return __prepare_proc(t, i->ei_class, elf, interp);
}
