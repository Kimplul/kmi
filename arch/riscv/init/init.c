#include <apos/init.h>
#include <apos/sizes.h>
#include <apos/types.h>
#include <apos/string.h>
#include <apos/debug.h>
#include <apos/pmem.h>
#include <apos/utils.h>
#include <libfdt.h>
#include <pages.h>
#include <csr.h>
#include <vmap.h>

struct pmem_layout {
	pm_t base;
	pm_t top;
};

struct cell_info {
	uint32_t size_cells;
	uint32_t addr_cells;
};

static struct cell_info get_cellinfo(void *fdt, int offset)
{
	struct cell_info ret = {
		fdt_size_cells(fdt, offset),
		fdt_address_cells(fdt, offset)
	};

	return ret;

}

static struct pmem_layout get_memlayout(void *fdt)
{
	struct cell_info ci = get_cellinfo(fdt, 0);

	int mem_offset = fdt_path_offset(fdt, "/memory");
	uint8_t *mem_reg =
		(uint8_t *) fdt_getprop(fdt, mem_offset, "reg", NULL);

	/* if riscv128 comes around we will probably see addr_cells == 4, but
	 * I'm not too concerned about it at the moment */
	pm_t base = fdt_load_int_ptr(pm_t, ci.addr_cells, mem_reg);

	if(ci.addr_cells == 2)
		mem_reg += sizeof(fdt64_t);
	else
		mem_reg += sizeof(fdt32_t);

	pm_t top = fdt_load_int_ptr(pm_t, ci.size_cells, mem_reg) + base;
	struct pmem_layout ret = { base, top };
	return ret;
}

#ifdef DEBUG

/* this should probably be improved in the future, possibly also moved
 * somewhere? */
static enum serial_dev_t serial_dev_enum(const char *dev_name)
{
	if (strncmp("ns16550", dev_name, 7) == 0)
		return NS16550A;

	return -1;
}

static void init_debug(void *fdt)
{
	int chosen_offset = fdt_path_offset(fdt, "/chosen");
	const char *stdout = fdt_getprop(fdt, chosen_offset,
			"stdout-path", NULL);

	int stdout_offset = fdt_path_offset(fdt, stdout);

	/* get serial device type */
	const char *dev_name = (const char *)fdt_getprop(fdt, stdout_offset,
			"compatible", NULL);

	enum serial_dev_t dev = serial_dev_enum(dev_name);

	/* get serial device address */
	struct cell_info ci = get_cellinfo(fdt, stdout_offset);
	void *reg_ptr = (void *)fdt_getprop(fdt, stdout_offset, "reg", NULL);

	void *uart_ptr = 0;
	uart_ptr = (void *)fdt_load_int_ptr(pm_t, ci.addr_cells, reg_ptr);

	dbg_init(uart_ptr, dev);
}

#else
#define init_debug(...)
#endif

static pm_t get_kerneltop()
{
	/* interesting, for some reason if I define these to be just char
	 * pointers I get some wacky values. Not sure why that would be, but
	 * this works. */
	extern char __init_end[], __kernel_size[];
	return (pm_t)__init_end + (pm_t)__kernel_size;
}

static pm_t get_initrdtop(void *fdt)
{
	int chosen_offset = fdt_path_offset(fdt, "/chosen");
	struct cell_info ci = get_cellinfo(fdt, chosen_offset);

	void *initrd_end_ptr = (void *)fdt_getprop(fdt, chosen_offset,
			"linux,initrd-end", NULL);

	return fdt_load_int_ptr(pm_t, ci.addr_cells, initrd_end_ptr);
}

static pm_t get_fdttop(void *fdt)
{
	const char *b = (const char *)fdt;
	return (pm_t)(b + fdt_totalsize(fdt));
}

static void setup_pmem(void *fdt)
{
	struct pmem_layout pmem = get_memlayout(fdt);

	pm_t initrd_top = get_initrdtop(fdt);
	pm_t kernel_top = get_kerneltop();
	pm_t fdt_top = get_fdttop(fdt);

	pm_t top = MAX3(kernel_top, initrd_top, fdt_top);
	dbg("initrd_top:\t%#lx\n", initrd_top);
	dbg("kernel_top:\t%#lx\n", kernel_top);
	dbg("fdt_top:\t%#lx\n", fdt_top);
}

void init(void *fdt)
{
	init_debug(fdt);
	dbg_fdt(fdt);
	setup_pmem(fdt);

	/* TODO: find first contiguous region */

	/* generate pagetable at contiguous region */
	/* populate_pmap(pmem.base, pmem.top - pmem.base, something); */

	/* TODO: mark all used pages */
	update_pmap(0/* TODO: figure out where in virtual memory the page map should be mapped */);
	/* TODO: jump to kernel */
}
