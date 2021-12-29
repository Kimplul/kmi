#include <apos/types.h>
#include <apos/attrs.h>
#include <apos/utils.h>
#include <vmem.h>
#include <csr.h>

/* assume 64 bit for now */
struct __packed init_vmem {
	int *leaf[512];
};

struct init_vmem *root_branch;

#define to_pte(a, f) (((a) >> 12) << 10 | (f))

/* assume Sv39 for now */
void init_vmem()
{
	size_t flags = VM_V | VM_X | VM_R | VM_W;

	extern char *__init_start;
	root_branch = (struct init_vmem *)align_down((size_t)&__init_start - SZ_4K, SZ_4K);
	for(size_t i = 0; i < 512; ++i)
		root_branch->leaf[i] = 0;

	/* direct mapping (temp) */
	for(size_t i = 0; i < 256; ++i)
		root_branch->leaf[i] = (int*)to_pte(SZ_1G * i, flags);

	/* kernel (also sort of direct mapping) */
	/* FIXME set up actually correct addressing retard */
	flags |= VM_G;
	for(size_t i = 256; i < 511; ++i)
		root_branch->leaf[i] = (int *)to_pte(RAM_BASE + SZ_1G * (i - 256), flags);

	/* kernel IO, map to 0 for now, should be made more robust in the future */
	/* same goes for the equivalent piece of code over in kernel/main.c,
	 * which btw should really get refactored, it's a mess */
	root_branch->leaf[511] = (int *)to_pte(0, flags);

	csr_write(CSR_SATP, SATP_MODE_Sv39 | ((size_t)root_branch >> 12));
}

void move_kernel()
{
	extern char *__init_end;
	extern char *__kernel_size;

	size_t sz = (size_t)&__kernel_size;
	char *src = (char *)&__init_end;
	char *dst = (char *)VM_KERN;
	for(size_t i = 0; i < sz; ++i)
		dst[i] = src[i];
}

void __init init(void *fdt)
{
	extern char *__init_end;
	void (*kernel_main)(void *fdt) = (void (*)(void *))(VM_KERN);
	init_vmem();
	move_kernel();
	kernel_main(fdt);
}
