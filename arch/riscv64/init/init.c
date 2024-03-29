/* SPDX-License-Identifier: copyleft-next-0.3.1 */
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
#include <libfdt.h>

#include "../kernel/csr.h"

void flush_tlb_full()
{
	/** @todo could be nice to have a common set of features with kernel,
	 * but for now this is good enough. */
	__asm__ volatile ("sfence.vma\n" ::: "memory");
}

/**
 * Create page table entry.
 * Copy from \ref arch/riscv64/kernel/vmem.c.
 *
 * @param a Virtual address.
 * @param f PTE flags.
 */
#define to_pte(a, f) (((a) >> 12) << 10 | (f))

/**
 * Get RAM base address from fdt.
 * @todo in case of multiple RAM banks, should try to just find one
 * of them and let the kernel figure the rest out. Kernel doesn't currently
 * support multiple RAM banks.
 *
 * @param fdt FDT pointer.
 * @return Physical address of ram base.
 */
static pm_t __fdt_ram_base(void *fdt)
{
	int mem_offset = fdt_path_offset(fdt, "/memory");
	const void *mem_reg = fdt_getprop(fdt, mem_offset, "reg", NULL);

	struct cell_info ci = get_cellinfo(fdt, mem_offset);
	return (pm_t)fdt_load_int_ptr(ci.addr_cells, mem_reg);
}

/**
 * Jump into virtual memory.
 *
 * @param load_addr Address where init has been loaded.
 * @param ram_base RAM base.
 */
static void init_bootmem(uintptr_t load_addr, uintptr_t ram_base)
{
	/* set all flags on, especially A and D since MMUs are allowed to raise
	 * exceptions that we're not ready to handle if they're unset. */
	size_t flags = VM_V | VM_X | VM_R | VM_W | VM_D | VM_A;

	extern char *__kernel;
	extern char *__kernel_size;
	uintptr_t top = load_addr + (uintptr_t)&__kernel +
	                (uintptr_t)&__kernel_size;

	/* this could be risky, as we might overwrite some bits of initrd or fdt
	 * if they're allocated too close to the kernel payload.
	 * @todo Allocate root_branch statically? */
	struct vmem *root_branch = (struct vmem *)align_up(top, SZ_4K);

	/* direct mapping (temp) */
	for (size_t i = 0; i <= CSTACK_PAGE; ++i)
		root_branch->leaf[i] = (struct vmem *)to_pte(TOP_PAGE_SIZE * i,
		                                             flags);

	/* kernel (also sort of direct mapping) */
	flags |= VM_G;
	for (size_t i = KSTART_PAGE; i < IO_PAGE; ++i)
		root_branch->leaf[i] = (struct vmem *)to_pte(
			ram_base + TOP_PAGE_SIZE * (i - KSTART_PAGE), flags);

	/* kernel IO, map to 0 for now, will be updated in the future */
	root_branch->leaf[IO_PAGE] = (struct vmem *)to_pte(0, flags);

	uintmax_t mode = Sv39;
	switch (DEFAULT_Sv_MODE) {
	case Sv32: mode = SATP_MODE_Sv32; break;
	case Sv39: mode = SATP_MODE_Sv39; break;
	case Sv48: mode = SATP_MODE_Sv48; break;
	default: break;
	};

	flush_tlb_full();
	csr_write(CSR_SATP, mode | ((uintptr_t)root_branch >> 12));
}

/**
 * Relocate kernel proper.
 * @param load_addr Address to where init has been loaded.
 * Used in calculating kernel start address.
 */
static void move_kernel(uintptr_t load_addr)
{
	extern char *__kernel;
	extern char *__kernel_size;

	size_t sz = (size_t)&__kernel_size;
	char *src = load_addr + (char *)&__kernel;
	char *dst = (char *)VM_KERN;
	for (size_t i = 0; i < sz; ++i)
		dst[i] = src[i];
}

/**
 * Main driver for the init loader.
 *
 * @param fdt Global FDT pointer, provided by bootloader.
 * @param load_addr Address to where init has been loaded.
 */
void init(void *fdt, pm_t load_addr)
{
	extern void jump_to_kernel(void *fdt, pm_t ram_base, void *k);

	pm_t ram_base = __fdt_ram_base(fdt);

	init_bootmem(load_addr, ram_base);
	move_kernel(load_addr);

	jump_to_kernel(fdt, ram_base, (void *)VM_KERN);
}

#if GENERIC_UBOOT
void init_go(int argc, char **argv, pm_t load_addr)
{
	if (argc != 2)
		return;

	void *fdt = (void *)strtouintptr(argv[1]);

	/* fdt is passed as second argument, load address first but since we
	 * already have it due to our ingenious _start, no need to parse it */
	init(fdt, load_addr);
}
#endif
