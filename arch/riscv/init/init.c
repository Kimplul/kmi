#include <apos/init.h>
#include <apos/sizes.h>
#include <apos/types.h>
#include <apos/string.h>
#include <apos/debug.h>
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

struct cell_info get_cellinfo(void *fdt, int offset)
{

	fdt32_t *size_ptr = 0;
	fdt32_t *addr_ptr = 0;

	while (offset) {
		size_ptr = (fdt32_t *) fdt_getprop(fdt, offset,
				"#size-cells", NULL);

		addr_ptr = (fdt32_t *) fdt_getprop(fdt, offset,
				"#address-cells", NULL);

		if (size_ptr && addr_ptr)
			break;

		/* dtc warns that this function is slow, but I don't care */
		offset = fdt_parent_offset(fdt, offset);
	}

	struct cell_info ret = {0};

	if (size_ptr && addr_ptr){
		ret.size_cells = fdt32_to_cpu(*size_ptr);
		ret.addr_cells = fdt32_to_cpu(*addr_ptr);
	}

	return ret;
}

struct mem_layout get_memlayout(void *fdt)
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
		top = fdt32_to_cpu(*(fdt64_t *) mem_reg) + base;

	struct mem_layout ret = { base, top };
	return ret;
}

#ifdef DEBUG

/* this should probably be improved in the future, possibly also moved
 * somewhere? */
enum serial_dev_t serial_dev_enum(const char *dev_name)
{
	if (strncmp("ns16550", dev_name, 7) == 0)
		return NS16550A;

	return -1;
}

void init_debug(void *fdt)
{
	int offset = fdt_path_offset(fdt, "/soc/uart");

	/* get serial device type */
	const char *dev_name = (const char *)fdt_getprop(fdt, offset,
			"compatible", NULL);

	enum serial_dev_t dev = serial_dev_enum(dev_name);

	/* get serial device address */
	struct cell_info ci = get_cellinfo(fdt, offset);
	void *reg_ptr = (void *)fdt_getprop(fdt, offset, "reg", NULL);

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

void init(void *fdt)
{
	init_debug(fdt);
	dbg("hello\n");

	struct mem_layout pmem = get_memlayout(fdt);
	/* struct mm_ptinfo ptbl = create_pagetable(pmem); */
}
