/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_ELF_H
#define APOS_ELF_H

/**
 * @file elf.h
 * ELF file handling.
 *
 * \todo Other file formats?
 */

#include <apos/attrs.h>
#include <apos/types.h>
#include <apos/vmem.h>

/** @name Magic. */
/** @{ */

/** Magic ELF header bytes. */
#define EI_MAGIC 0x7f454c46

/** @} */

/** @name ELF classification. */
/** @{ */

/** ELF class none, likely invalid binary. */
#define ELFCLASSNONE 0x0

/** ELF class 32, 32bit binary. */
#define ELFCLASS32 0x1

/** ELF class 64, 64bit binary. */
#define ELFCLASS64 0x2

/** @} */

/** @name ELF data formats. */
/** @{ */

/** ELF data none, likely invalid binary. */
#define ELFDATANONE 0x0

/** ELF data little endian. */
#define ELFDATA2LSB 0x1

/** ELF data big endian. */
#define ELFDATA2MSB 0x2

/** @} */

/**
 * @name ELF ABIs.
 * Missing apos, of course, not sure which of these I should use instead. Some
 * BSD?
 */
/** @{ */

/** ELF operating system ABI none. Likely standalone binary. */
#define ELFOSABI_NONE 0x0

/** ELF UNIX SystemV ABI. */
#define ELFOSABI_SYSV 0x0

/** ELF NetBSD ABI. */
#define ELFOSABI_NETBSD 0x2

/** ELF Linux ABI. */
#define ELFOSABI_LINUX 0x3

/** ELF GNU Hurd ABI. */
#define ELFOSABI_HURD 0x4

/** ELF FreeBSD ABI. */
#define ELFOSABI_FREEBSD 0x9

/** ELF OpenBSD ABI. */
#define ELFOSABI_OPENBSD 0xc

/** @} */

/** @name ELF binary types. */
/** @{ */

/** Binary type none. Likely invalid binary. */
#define ET_NONE 0x0

/** Relocatable file. */
#define ET_REL 0x1

/** Executable file. */
#define ET_EXEC 0x2

/** Shared object file. */
#define ET_DYN 0x3

/** Core file. */
#define ET_CORE 0x4

/** @} */

/**
 * @name Machine architecture.
 * Currently only RISCV supported.
 */
/** @{ */

/** Riscv. */
#define EM_RISCV 0xf3

/** @} */

/** ELF identification data. */
struct __packed elf_ident {
	/** Magic. \see EI_MAGIC. */
	uint32_t ei_magic;

	/** ELF class. \see ELFCLASSNONE, ELFCLASS32, ELFCLASS64. */
	uint8_t ei_class;

	/** ELF data formats. \see ELFDATANONE, ELFDATA2LSB, ELFDATA2MSB. */
	uint8_t ei_data;

	/** ELF version. \c 1 for current, \c 0 for undefined. */
	uint8_t ei_version;

	/** ELF ABI. \see ELFOSABI_NONE, ELFOSABI_SYSV, ELFOSABI_NETBSD,
	 * ELFOSABI_LINUX, ELFOSABI_HURD, ELFOSABI_FREEBSD. */
	uint8_t ei_osabi;

	/** ELF ABI version. Depends on \c ei_osabi. */
	uint8_t ei_abiversion;

	/** Padding. */
	uint8_t ei_pad[7];
};

/** ELF32 header for ELF binaries of class \ref ELFCLASS32. */
struct __packed elf32_header {
	/** Common identity header. */
	struct elf_ident e_ident;

	/** ELF type. \see ET_NONE, ET_REL, ET_EXEC, ET_DYN, ET_CORE. */
	uint16_t e_type;

	/** Machine architecture. \see EM_RISCV. */
	uint16_t e_machine;

	/** ELF version. \c 1 for current, \c 0 for undefined. */
	uint32_t e_version;

	/** Entrypoint virtual address. */
	uint32_t e_entry;

	/** Start of program header table within binary image. */
	uint32_t e_phoff;

	/** Start of section header table within binary image. */
	uint32_t e_shoff;

	/** Architecture dependent flags. */
	uint32_t e_flags;

	/** Size of this header. 52 bytes for ELF32. */
	uint16_t e_ehsize;

	/** Size of one program header table entry. */
	uint16_t e_phentsize;

	/** Number of program header table entries. */
	uint16_t e_phnum;

	/** Size of one section header table entry. */
	uint16_t e_shentsize;

	/** Number of section header table entries. */
	uint16_t e_shnum;

	/** Index into section header table where section names are kept. */
	uint16_t e_shstrndx;
};

/** ELF64 header for ELF binaries of class \ref ELFCLASS64. */
struct __packed elf64_header {
	/** Common identity header. */
	struct elf_ident e_ident;

	/** ELF type. \see ET_NONE, ET_REL, ET_EXEC, ET_DYN, ET_CORE. */
	uint16_t e_type;

	/** Machine architecture. \see EM_RISCV. */
	uint16_t e_machine;

	/** ELF version. \c 1 for current, \c 0 for undefined. */
	uint32_t e_version;

	/** Entrypoint virtual address. */
	uint64_t e_entry;

	/** Start of program header table within binary image. */
	uint64_t e_phoff;

	/** Start of section header table within binary image. */
	uint64_t e_shoff;

	/** Architecture dependent flags. */
	uint32_t e_flags;

	/** Size of this header. 64 bytes for ELF64. */
	uint16_t e_ehsize;

	/** Size of one program header table entry. */
	uint16_t e_phentsize;

	/** Number of program header table entries. */
	uint16_t e_phnum;

	/** Size of one section header table entry. */
	uint16_t e_shentsize;

	/** Number of section header table entries. */
	uint16_t e_shnum;

	/** Index into section header table where section names are kept. */
	uint16_t e_shstrndx;
};

/** @name Program header table entry types. */
/** @{ */

/** Null header type. */
#define PT_NULL 0x0

/** Load header type. */
#define PT_LOAD 0x1

/** Dynamic header type. */
#define PT_DYNAMIC 0x2

/** Interpreter header type. */
#define PT_INTERP 0x3

/** Note header type. */
#define PT_NOTE 0x4

/** Reserved, unsepcifier header type. */
#define PT_SHLIB 0x5

/** Program header table header type. */
#define PT_PHDR 0x6

/** Thread local storage header type. */
#define PT_TLS 0x7

/** Operating system specific low fence. */
#define PT_LOOS 0x60000000

/** Operating system specific high fence. */
#define PT_HIOS 0x6fffffff

/** Processor specific low fence. */
#define PT_LOPROC 0x70000000

/** Processor specific high fence. */
#define PT_HIPROC 0x7fffffff

/** @} */

/** @name Program header entry access types. */
/** @{ */

/** Executable. */
#define PF_X (1 << 0)

/** Writable. */
#define PF_W (1 << 1)

/** Readable. */
#define PF_R (1 << 2)

/** @} */

/** ELF32 program header table entry. \see ELFCLASS32. */
struct __packed program32_header {
	/** Program header entry type. \see PT_NULL, PT_LOAD, PT_DYNAMIC,
	 * PT_INTERP, PT_NOTE, PT_SHLIB, PT_PHDR, PT_TLS, PT_LOOS, PT_HIOS,
	 * PT_LOPROC, PT_HIPROC. */
	uint32_t p_type;

	/** Offset in the binary image where the associated segment lies. */
	uint32_t p_offset;

	/** Virtual address to first byte in segment. */
	uint32_t p_vaddr;

	/** Physical address to first byte in segment. Currently ignored. */
	uint32_t p_paddr;

	/** Segment size as bytes in binary image. */
	uint32_t p_filesz;

	/** Segment size as bytes in memory. */
	uint32_t p_memsz;

	/** Access flags for segment. \see PF_X, PF_W, PF_R. */
	uint32_t p_flags;

	/** Alignment of segment. */
	uint32_t p_align;
};

/** ELF64 program header table entry. \see ELFCLASS64. */
struct __packed program64_header {
	/** Program header entry type. \see PT_NULL, PT_LOAD, PT_DYNAMIC,
	 * PT_INTERP, PT_NOTE, PT_SHLIB, PT_PHDR, PT_TLS, PT_LOOS, PT_HIOS,
	 * PT_LOPROC, PT_HIPROC. */
	uint32_t p_type;

	/** Access flags for segment. \see PF_X, PF_W, PF_R. */
	uint32_t p_flags;

	/** Offset in the binary image where the associated segment lies. */
	uint64_t p_offset;

	/** Virtual address to first byte in segment. */
	uint64_t p_vaddr;

	/** Physical address to first byte in segment. Currently ignored. */
	uint64_t p_paddr;

	/** Segment size as bytes in binary image. */
	uint64_t p_filesz;

	/** Segment size as bytes in memory. */
	uint64_t p_memsz;

	/** Alignment of segment. */
	uint64_t p_align;
};

/** @name Section header types. */
/** @{ */

/** Null type. */
#define SHT_NULL 0x0

/** Section contains information defined by the program. */
#define SHT_PROGBITS 0x1

/** Section contains symbol table. */
#define SHT_SYMTAB 0x2

/** Section contains string table. */
#define SHT_STRTAB 0x3

/** Section contains relocation table with explicit addends. */
#define SHT_RELA 0x4

/** Section contains hashes. */
#define SHT_HASH 0x5

/** Section contains dynamic linking information. */
#define SHT_DYNAMIC 0x6

/** Section contains notes. */
#define SHT_NOTE 0x7

/** Section occupies no space in binary image. */
#define SHT_NOBITS 0x8

/** Section contains relocation table. */
#define SHT_REL 0x9

/** Reserved, unspecified semantincs. */
#define SHT_SHLIB 0x0a

/** Sections hold symbol table. */
#define SHT_DYNSYM 0x0b

/** Section contains array of pointers to initialization functions. */
#define SHT_INIT_ARRAY 0x0e

/** Section contains array of pointers to finalization functions. */
#define SHT_FINI_ARRAY 0x0f

/** Section contains array of pointers to preinitialization functions. */
#define SHT_PREINIT_ARRAY 0x10

/** Section group. */
#define SHT_GROUP 0x11

/** Section contains extended symbol table index. */
#define SHT_SYMTAB_SHNDX 0x12

/** Number of reserved SHT_* values. */
#define SHT_NUM 0x13

/** @} */

/** @name Section header flags. */
/** @{ */

/** Section should be writable during execution. */
#define SHF_WRITE 0x1

/** Section occupies memory during execution. */
#define SHF_ALLOC 0x2

/** Section contains executable machine instructions. */
#define SHF_EXECINSTR 0x4

/** Section might be merged. */
#define SHF_MERGE 0x10

/** Section contains null-terminated strings. */
#define SHF_STRINGS 0x20

/** Section contains section header table index. */
#define SHF_INFO_LINK 0x40

/** Preserve order after combining. */
#define SHF_LINK_ORDER 0x80

/** Non-standard OS specific handling. */
#define SHF_OS_NONCONFORMING 0x100

/** Section is a member of group. */
#define SHF_GROUP 0x200

/** Section hold thread local data. */
#define SHF_TLS 0x400

/** Operating system mask. */
#define SHF_MASKOS 0x0ff00000

/** Processor mask. */
#define SHF_MASKPROC 0xf0000000

/** @} */

/** ELF32 section header table entry. \see ELFCLASS32. */
struct __packed section32_header {
	/** Section header name. */
	uint32_t sh_name;

	/** Section header type. \see SHT_NULL, SHT_PROGBITS, SHT_SYMTAB,
	 * SHT_STRTAB, SHT_RELA, SHT_HASH, SHT_DYNAMIC, SHT_NOTE, SHT_NOBITS,
	 * SHT_REL, SHT_SHLIB, SHT_DYNSYM, SHT_INIT_ARRAY, SHT_FINI_ARRAY,
	 * SHT_PREINIT_ARRAY, SHT_GROUP, SHT_SYMTAB_SHNDX, SHT_NUM. */
	uint32_t sh_type;

	/** Section header flags. \see SHF_WRITE, SHF_ALLOC, SHF_EXECINSTR,
	 * SHF_MERGE, SHF_STRINGS, SHF_INFO_LINK, SHF_LINK_ORDER,
	 * SHF_OS_NONCONFORMING, SHF_GROUP, SHF_TLS, SHF_MASKOS, SHF_MASKPROC.
	 */
	uint32_t sh_flags;

	/** Section virtual address. */
	uint32_t sh_addr;

	/** Offset of section in binary image. */
	uint32_t sh_offset;

	/** Size as bytes in binary image. */
	uint32_t sh_size;

	/** Interpretation depends on section header type. */
	uint32_t sh_link;

	/** Interpretation depends on section header type. */
	uint32_t sh_info;

	/** Alignment of section address. */
	uint32_t sh_addralign;

	/** If section holds an array, each entry is of this size. */
	uint32_t sh_entsize;
};

/** ELF64 section header table entry. \see ELFCLASS64. */
struct __packed section64_header {
	/** Section header name. */
	uint32_t sh_name;

	/** Section header type. \see SHT_NULL, SHT_PROGBITS, SHT_SYMTAB,
	 * SHT_STRTAB, SHT_RELA, SHT_HASH, SHT_DYNAMIC, SHT_NOTE, SHT_NOBITS,
	 * SHT_REL, SHT_SHLIB, SHT_DYNSYM, SHT_INIT_ARRAY, SHT_FINI_ARRAY,
	 * SHT_PREINIT_ARRAY, SHT_GROUP, SHT_SYMTAB_SHNDX, SHT_NUM. */
	uint32_t sh_type;

	/** Section header flags. \see SHF_WRITE, SHF_ALLOC, SHF_EXECINSTR,
	 * SHF_MERGE, SHF_STRINGS, SHF_INFO_LINK, SHF_LINK_ORDER,
	 * SHF_OS_NONCONFORMING, SHF_GROUP, SHF_TLS, SHF_MASKOS, SHF_MASKPROC.
	 */
	uint64_t sh_flags;

	/** Section virtual address. */
	uint64_t sh_addr;

	/** Offset of section in binary image. */
	uint64_t sh_offset;

	/** Size as bytes in binary image. */
	uint64_t sh_size;

	/** Interpretation depends on section header type. */
	uint32_t sh_link;

	/** Interpretation depends on section header type. */
	uint32_t sh_info;

	/** Alignment of section address. */
	uint64_t sh_addralign;

	/** If section holds an array, each entry is of this size. */
	uint64_t s_entsize;
};

/**
 * Get ELF identity from pointer.
 *
 * @param e Pointer to ELF identity.
 * @return \c e as ELF identity.
 * \see elf_ident.
 */
#define elf_indent(e) ((struct elf_ident *)e)

/**
 * Get ELF64 header from pointer.
 *
 * @param e Pointer to ELF64 header.
 * @return \c e as ELF64 header.
 * \see elf64_header.
 */
#define elf64_header(e) ((struct elf64_header *)e)

/**
 * Get ELF32 header from pointer.
 *
 * @param e Pointer to ELF32 header.
 * @return \c e as ELF32 header.
 * \see elf32_header.
 */
#define elf32_header(e) ((struct elf32_header *)e)

/** Get ELF64 program header from pointer.
 *
 * @param p Pointer to ELF64 program header.
 * @return \c p as ELF64 program header.
 * \see program64_header.
 */
#define program64_header(p) ((struct program64_header *)p)

/**
 * Get ELF32 program header from pointer.
 *
 * @param p Pointer to ELF32 program header.
 * @return \c p as ELF32 program header.
 * \see program32_header.
 */
#define program32_header(p) ((struct program32_header *)p)

/**
 * Get ELF64 section header from pointer.
 *
 * @param s Pointer to ELF64 section header.
 * @return \c s as ELF64 section header.
 * \see section64_header.
 */
#define section64_header(s) ((struct section64_header *)s)

/**
 * Get ELF32 section header from pointer.
 *
 * @param s Pointer to ELF32 section header.
 * @return \c s as ELF32 section header.
 * \see section32_header.
 */
#define section32_header(s) ((struct section32_header *)s)

/**
 * Get either ELF32 or ELF64 header property, depending on the class.
 *
 * @param c ELF class. \see ELFCLASS32, ELFCLASS64.
 * @param e ELF header. \see elf32_header, elf64_header.
 * @param p Property to get.
 * @return \c p
 */
#define elf_header_prop(c, e, p) \
	(c == ELFCLASS64 ? elf64_header(e)->p : elf32_header(e)->p)

/**
 * Get Either ELF32 or ELF64 program header property, depending on the class.
 *
 * @param c ELF class. \see ELFCLASS32, ELFCLASS64.
 * @param e ELF program header. \see program32_header, program64_header.
 * @param p Property to get.
 * @return \c p.
 */
#define program_header_prop(c, e, p) \
	(c == ELFCLASS64 ? program64_header(e)->p : program32_header(e)->p)

/**
 * Get either ELF32 or ELF64 section header property, depending on the class.
 *
 * @param c ELF class. \see ELFCLASS32, ELFCLASS64.
 * @param e ELF section header. \see section32_header, section64_header.
 * @param p Property to get.
 * @return \c p.
 */
#define section_header_prop(c, e, p) \
	(c == ELFCLASS64 ? section64_header(e)->p : section32_header(e)->p)

/**
 * Build up ELF memory image from binary image.
 *
 * @param p Process space in which to build the memory image.
 * @param binary Virtual address of binary.
 * @param interp Virtual address of optional interpreter.
 * @return Virtual address of entry point.
 */
vm_t load_elf(struct tcb *p, vm_t binary, vm_t interp);

#endif /* APOS_ELF_H */
