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

struct mem_layout {
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

static struct mem_layout get_memlayout(void *fdt)
{
	struct cell_info ci = get_cellinfo(fdt, 0);

	int mem_offset = fdt_path_offset(fdt, "/memory");
	uint8_t *mem_reg =
		(uint8_t *) fdt_getprop(fdt, mem_offset, "reg", NULL);

	pm_t base;
	/* if riscv128 comes around we will probably see addr_cells == 4, but
	 * I'm not too concerned about it at the moment */
	if (ci.addr_cells == 2) {
		base = fdt64_to_cpu(*(fdt64_t *) mem_reg);
		mem_reg += sizeof(fdt64_t);
	} else {
		base = fdt32_to_cpu(*(fdt32_t *) mem_reg);
		mem_reg += sizeof(fdt32_t);
	}

	pm_t top;
	if (ci.size_cells == 2)
		top = fdt64_to_cpu(*(fdt64_t *) mem_reg) + base;
	else
		top = fdt32_to_cpu(*(fdt32_t *) mem_reg) + base;

	struct mem_layout ret = { base, top };
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
	const char *stdout = fdt_getprop(fdt, chosen_offset, "stdout-path", 0);

	int stdout_offset = fdt_path_offset(fdt, stdout);

	/* get serial device type */
	const char *dev_name = (const char *)fdt_getprop(fdt, stdout_offset,
			"compatible", NULL);

	enum serial_dev_t dev = serial_dev_enum(dev_name);

	/* get serial device address */
	struct cell_info ci = get_cellinfo(fdt, stdout_offset);
	void *reg_ptr = (void *)fdt_getprop(fdt, stdout_offset, "reg", NULL);

	void *uart_ptr = 0;
	if (ci.addr_cells == 2)
		uart_ptr = (void *)(pm_t)fdt64_to_cpu(*(fdt64_t *) reg_ptr);
	else
		uart_ptr = (void *)(pm_t)fdt32_to_cpu(*(fdt32_t *) reg_ptr);

	dbg_init(uart_ptr, dev);
}

#else
#define init_debug(...)
#endif

/* TODO: these */
static pm_t get_kerneltop(void *fdt)
{
	return 0;
}

static pm_t get_initrdtop(void *fdt)
{
	return 0;
}

static pm_t get_fdttop(void *fdt)
{
	return 0;
}

static void setup_pmem(void *fdt)
{
	struct mem_layout pmem = get_memlayout(fdt);

	pm_t kernel_top = get_kerneltop(fdt);
	pm_t initrd_top = get_initrdtop(fdt);
	pm_t fdt_top = get_fdttop(fdt);

	pm_t top = MAX3(kernel_top, initrd_top, fdt_top);
}

void __noreturn init(void *fdt)
{
	init_debug(fdt);
	dbg_fdt(fdt);
	setup_pmem(fdt);

	/* basic memory layout info */
	struct mem_layout pmem = get_memlayout(fdt);

	/* TODO: find first contiguous region */

	/* generate pagetable at contiguous region */
	populate_pmap(pmem.base, pmem.top - pmem.base, 0/* something */);

	/* TODO: mark all used pages */
	update_pmap(0/* TODO: figure out where in virtual memory the page map should be mapped */);
	/* TODO: jump to kernel */
}
