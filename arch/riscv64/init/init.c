/**
 * @file init.c
 * riscv64 'bootstrap', move actual kernel into place and jump to virtual
 * memory.
 */

#include <apos/types.h>
#include <apos/attrs.h>
#include <apos/utils.h>
#include <apos/vmem.h>
#include <vmem.h>
#include <csr.h>

/* assume 64 bit for now */
struct vmem *root_branch;

#define to_pte(a, f) (((a) >> 12) << 10 | (f))

/* assume Sv39 for now */
void init_bootmem()
{
	size_t flags = VM_V | VM_X | VM_R | VM_W;

	extern char *__init_start;
	root_branch = (struct vmem *)align_down(
		(uintptr_t)&__init_start - SZ_4K, SZ_4K);

	/* direct mapping (temp) */
	for (size_t i = 0; i < CSTACK_PAGE; ++i)
		root_branch->leaf[i] = (struct vmem *)to_pte(SZ_1G * i, flags);

	/* kernel (also sort of direct mapping) */
	flags |= VM_G;
	for (size_t i = KSTART_PAGE; i < IO_PAGE; ++i)
		root_branch->leaf[i] = (struct vmem *)to_pte(
			RAM_BASE + SZ_1G * (i - 256), flags);

	/* kernel IO, map to 0 for now, will be updated in the future */
	root_branch->leaf[IO_PAGE] = (struct vmem *)to_pte(0, flags);

	csr_write(CSR_SATP, SATP_MODE_Sv39 | ((uintptr_t)root_branch >> 12));
}

void move_kernel()
{
	extern char *__init_end;
	extern char *__kernel_size;

	unsigned long sz = (unsigned long)&__kernel_size;
	char *src = (char *)&__init_end;
	char *dst = (char *)VM_KERN;
	for (size_t i = 0; i < sz; ++i)
		dst[i] = src[i];
}

#define __va_reg(reg)                                                          \
	{                                                                      \
		vm_t reg = 0;                                                  \
		__asm__ ("mv %0, " QUOTE(reg) : "=r" (reg)::);                   \
		reg = (vm_t)__va(reg);                                         \
		__asm__ ("mv " QUOTE(reg) ", %0" ::"rK" (reg) :);                \
	}

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
