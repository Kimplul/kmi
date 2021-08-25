#include <apos/init.h>
#include <apos/sizes.h>
#include <apos/types.h>
#include <libfdt.h>
#include <pages.h>
#include <csr.h>
#include <vmap.h>

struct mem_layout {
	pm_t base;
	pm_t top;
};

struct mem_layout get_memlayout(void *fdt)
{
	fdt32_t *size_ptr = (fdt32_t *)fdt_getprop(fdt, 0, "#size-cells", NULL);
	fdt32_t *addr_ptr = (fdt32_t *)fdt_getprop(fdt, 0, "#address-cells", NULL);

	uint32_t size_cells = fdt32_to_cpu(*size_ptr);
	uint32_t addr_cells = fdt32_to_cpu(*addr_ptr);

	int mem_offset = fdt_path_offset(fdt, "/memory");
	uint8_t *mem_reg = (uint8_t *)fdt_getprop(fdt, mem_offset, "reg", NULL);

	pm_t base;
	/* if riscv128 comes around we will probably see addr_cells == 4, but
	 * I'm not too concerned about it at the moment */
	if (addr_cells == 2) {
		base = fdt64_to_cpu(*(fdt64_t *)mem_reg);
		mem_reg += sizeof(fdt64_t);
	} else {
		base = fdt32_to_cpu(*(fdt32_t *)mem_reg);
		mem_reg += sizeof(fdt32_t);
	}

	pm_t top;
	if (size_cells == 2)
		top = fdt64_to_cpu(*(fdt64_t *)mem_reg) + base;
	else
		top = fdt32_to_cpu(*(fdt64_t *)mem_reg) + base;

	struct mem_layout ret = {base, top};
	return ret;
}

void init(void *fdt)
{
	struct mem_layout pmem = get_memlayout(fdt);
	/* struct mm_ptinfo ptbl = create_pagetable(pmem); */
}
