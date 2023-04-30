/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file init.c
 * riscv64 'bootstrap', move actual kernel into place and jump to virtual
 * memory.
 */

#include <kmi/types.h>
#include <kmi/attrs.h>
#include <kmi/utils.h>
#include <kmi/vmem.h>
#include <arch/vmem.h>
#include "../kernel/csr.h"

/** Temporary virtual memory space.
 * Assume 64bit riscv for now.
 */
struct vmem *root_branch;

/**
 * Create page table entry.
 * Copy from \ref arch/riscv64/kernel/vmem.c.
 *
 * @param a Virtual address.
 * @param f PTE flags.
 */
#define to_pte(a, f) (((a) >> 12) << 10 | (f))

/** Jump into virtual memory. */
static void init_bootmem()
{
	size_t flags = VM_V | VM_X | VM_R | VM_W;

	extern char *__init_end;
	extern char *__kernel_size;
	uintptr_t top = (uintptr_t)&__init_end + (uintptr_t)&__kernel_size;

	/* this could be risky, as we might overwrite some bits of initrd or fdt
	 * if they're allocated too close to the kernel payload.
	 * @todo Allocate root_branch statically? */
	root_branch = (struct vmem *)align_up(top, SZ_4K);

	/* direct mapping (temp) */
	for (size_t i = 0; i < CSTACK_PAGE; ++i)
		root_branch->leaf[i] = (struct vmem *)to_pte(TOP_PAGE_SIZE * i,
		                                             flags);

	/* kernel (also sort of direct mapping) */
	flags |= VM_G;
	for (size_t i = KSTART_PAGE; i < IO_PAGE; ++i)
		root_branch->leaf[i] = (struct vmem *)to_pte(
			RAM_BASE + TOP_PAGE_SIZE * (i - KSTART_PAGE), flags);

	/* kernel IO, map to 0 for now, will be updated in the future */
	root_branch->leaf[IO_PAGE] = (struct vmem *)to_pte(0, flags);

	uintmax_t mode = Sv39;
	switch (DEFAULT_Sv_MODE) {
	case Sv32: mode = SATP_MODE_Sv32; break;
	case Sv39: mode = SATP_MODE_Sv39; break;
	case Sv48: mode = SATP_MODE_Sv48; break;
	default: break;
	};
	csr_write(CSR_SATP, mode | ((uintptr_t)root_branch >> 12));
}

/** Relocate kernel proper. */
static void move_kernel()
{
	extern char *__init_end;
	extern char *__kernel_size;

	unsigned long sz = (unsigned long)&__kernel_size;
	char *src = (char *)&__init_end;
	char *dst = (char *)VM_KERN;
	for (size_t i = 0; i < sz; ++i)
		dst[i] = src[i];
}

/**
 * Convert an existing physical address in a register to a virtual address.
 * There is probably an easier way to do this, but this seems to work alright.
 *
 * @param reg Register to modify.
 */
#define __va_reg(reg)                                             \
	{                                                         \
		vm_t reg = 0;                                     \
		__asm__ ("mv %0, " QUOTE(reg) : "=r" (reg)::);    \
		reg = (vm_t)__va(reg);                            \
		__asm__ ("mv " QUOTE(reg) ", %0" ::"rK" (reg) :); \
	}

/**
 * Main driver for the init loader.
 *
 * @param fdt Global FDT pointer, provided by bootloader.
 */
void init(void *fdt)
{
	extern char *__init_end;
	extern void jump_to_kernel(void *k, void *fdt);

	init_bootmem();
	move_kernel();
	__va_reg(sp);
	__va_reg(fp);
	__va_reg(gp);

	jump_to_kernel((void *)VM_KERN, __va(fdt));
}
