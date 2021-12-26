#ifndef APOS_ELF_H
#define APOS_ELF_H

#include <apos/attrs.h>
#include <apos/types.h>
#include <apos/vmem.h>

#define EI_MAGIC 0x7f454c46

#define ELFCLASSNONE		0x0
#define ELFCLASS32		0x1
#define ELFCLASS64		0x2

#define ELFDATANONE		0x0
#define ELFDATA2LSB		0x1
#define ELFDATA2MSB		0x2

#define ELFOSABI_NONE		0x0
#define ELFOSABI_SYSV		0x0
#define ELFOSABI_NETBSD		0x2
#define ELFOSABI_LINUX		0x3
#define ELFOSABI_HURD		0x4
#define ELFOSABI_FREEBSD	0x9
#define ELFOSABI_OPENBSD	0xc

#define ET_NONE			0x0
#define ET_REL			0x1
#define ET_EXEC			0x2
#define ET_DYN			0x3
#define ET_CORE			0x4

#define EM_RISCV		0xf3

__packed struct elf_ident {
	uint32_t ei_magic;
	uint8_t ei_class;
	uint8_t ei_data;
	uint8_t ei_version;
	uint8_t ei_osabi;
	uint8_t ei_abiversion;
	uint8_t ei_pad[7];
};

__packed struct elf32_header {
	struct elf_ident e_ident;
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

__packed struct elf64_header {
	struct elf_ident e_ident;
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint64_t e_entry;
	uint64_t e_phoff;
	uint64_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

#define PT_NULL			0x0
#define PT_LOAD			0x1
#define PT_DYNAMIC		0x2
#define PT_INTERP		0x3
#define PT_NOTE			0x4
#define PT_SHLIB		0x5
#define PT_PHDR			0x6
#define PT_TLS			0x7
#define PT_LOOS			0x60000000
#define PT_HIOS			0x6fffffff
#define PT_LOPROC		0x70000000
#define PT_HIPROC		0x7fffffff

#define PF_X			(1 << 0)
#define PF_W			(1 << 1)
#define PF_R			(1 << 2)

__packed struct program32_header {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
};

__packed struct program64_header {
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
};

#define SHT_NULL		0x0
#define SH_PROGBITS		0x1
#define SHT_SYMTAB		0x2
#define SHT_STRTAB		0x3
#define SHT_RELA		0x4
#define SHT_HASH		0x5
#define SHT_DYNAMIC		0x6
#define SHT_NOTE		0x7
#define SHT_NOBITS		0x8
#define SHT_REL			0x9
#define SHT_SHLIB		0x0a
#define SHT_DYNSYM		0x0b
#define SHT_INIT_ARRAY		0x0e
#define SHT_FINI_ARRAY		0x0f
#define SHT_PREINIT_ARRAY	0x10
#define SHT_GROUP		0x11
#define SHT_SYMTAB_SHNDX	0x12
#define SHT_NUM			0x13

#define SHF_WRITE		0x1
#define SHF_ALLOC		0x2
#define SHF_EXECINSTR		0x4
#define SHF_MERGE		0x10
#define SHF_STRINGS		0x20
#define SHF_INFO_LINK		0x40
#define SHF_LINK_ODER		0x80
#define SHF_OS_NONCONFORMING	0x100
#define SHF_GROUP		0x200
#define SHF_TLS			0x400
#define SHF_MASKOS		0x0ff00000
#define SHF_MASKPROC		0xf0000000

__packed struct section32_header {
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
};

__packed struct section64_header {
	uint32_t sh_name;
	uint32_t sh_type;
	uint64_t sh_flags;
	uint64_t sh_addr;
	uint64_t sh_offset;
	uint64_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint64_t sh_addralign;
	uint64_t s_entsize;
};

#define elf_indent(e) ((struct elf_ident *)e)
#define elf64_header(e) ((struct elf64_header *)e)
#define elf32_header(e) ((struct elf32_header *)e)
#define program64_header(p) ((struct program64_header *)p)
#define program32_header(p) ((struct program32_header *)p)
#define section64_header(s) ((struct section64_header *)s)
#define section32_header(s) ((struct section32_header *)s)

#define elf_header_prop(c, e, p) (c == ELFCLASS64 ? elf64_header(e)->p : elf32_header(e)->p)
#define program_header_prop(c, e, p) (c == ELFCLASS64 ? program64_header(e)->p : program32_header(e)->p)
#define section_header_prop(c, e, p) (c == ELFCLASS64 ? section64_header(e)->p : section32_header(e)->p)

vm_t prepare_proc(struct tcb *t, vm_t p);

#endif /* APOS_ELF_H */
