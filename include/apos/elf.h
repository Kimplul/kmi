#ifndef APOS_ELF_H
#define APOS_ELF_H

#include <apos/attrs.h>
#include <apos/types.h>
#include <apos/vmem.h>

__packed struct elf_header {
	uint64_t ei_magic;
	uint8_t ei_class;
	uint8_t ei_data;
	uint8_t ei_version;
	uint8_t ei_osabi;
	uint8_t ei_abiversion;
	uint8_t e_pad;
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	void *e_entry;
	void *e_phoff;
	void *e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

vm_t bin_entry(vm_t bin);

#endif /* APOS_ELF_H */
