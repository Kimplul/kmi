#include <apos/types.h>
#include <apos/mem.h>
#include <libfdt.h>

size_t __mm_shifts[10];
size_t __mm_widths[10];
size_t __mm_sizes[10];
size_t __mm_page_shift;
size_t __mm_max_order;

void init_mem(size_t max_order, size_t widths[10], size_t page_shift)
{
	__mm_max_order = max_order;
	__mm_page_shift = page_shift;

	__mm_shifts[0] = 0;
	__mm_widths[0] = 1 << widths[0];
	__mm_sizes[0]  = 1 << __mm_page_shift;

	for(size_t i = 0; i <= __mm_max_order; ++i){
		__mm_widths[i] = 1 << widths[i];
		__mm_shifts[i] = __mm_shifts[i - 1] + __mm_widths[i - 1];
		__mm_sizes[i] = 1 << __mm_shifts[i] << __mm_page_shift;
	}
}

enum mm_mode_t get_mmode(void *fdt)
{
	int mmu_offset = fdt_path_offset(fdt, "/cpus/cpu");
	const char *mmu = fdt_getprop(fdt, mmu_offset, "mmu-type", NULL);

	if(strncmp("riscv,sv48", mmu, 10) == 0)
		return Sv48;

	if(strncmp("riscv,sv39", mmu, 10) == 0)
		return Sv39;

	if(strncmp("riscv,sv32", mmu, 10) == 0)
		return Sv32;

	/* fdt is missing mmu-type for some reason, but we can probably use
	 * these values as fallback */
	if(__riscv_xlen == 32)
		return Sv32;
	else
		return Sv39;
}
