#include <apos/types.h>
#include <apos/attrs.h>
#include <vmem.h>
#include <csr.h>

/* assume 64 bit for now */
struct __packed init_vmem {
	int *leaf[512];
};

struct init_vmem root_branch;

/* assume Sv48 for now */
void init_lowmem()
{
	size_t flags = VM_V | VM_X | VM_R | VM_W;
	for(size_t i = 0; i < 512; ++i)
		root_branch.leaf[i] = (int *)((1UL << 37) * i | flags);

	 csr_write(CSR_SATP, SATP_MODE_Sv48 | ((size_t)&root_branch >> 12));
}

void __section(".init.start") main(void *fdt)
{
	extern char *__init_end;
	void (*kernel_main)(void *fdt) = (void (*)(void *))(VM_KERN + (size_t)&__init_end);
	init_lowmem();
	kernel_main(fdt);
}
