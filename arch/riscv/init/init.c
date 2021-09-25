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

/* How "reg" is interpreted depends on the parent node */
static struct cell_info get_reginfo(void *fdt, const char *path)
{
	const char *i = strrchr(path, '/');
	if(!i)
		return (struct cell_info){0, 0};

	size_t baselen = i - path;
	if(i == 0)
		/* root node */
		baselen = 1;

	return get_cellinfo(fdt, fdt_path_offset_namelen(fdt, path, baselen));
}

static struct pmem_layout get_memlayout(void *fdt)
{
	struct cell_info ci = get_reginfo(fdt, "/memory");
	int mem_offset = fdt_path_offset(fdt, "/memory");
	uint8_t *mem_reg =
		(uint8_t *) fdt_getprop(fdt, mem_offset, "reg", NULL);

	/* if riscv128 comes around we will probably see addr_cells == 4, but
	 * I'm not too concerned about it at the moment */
	pm_t base = (pm_t)fdt_load_int_ptr(ci.addr_cells, mem_reg);

	if(ci.addr_cells == 2)
		mem_reg += sizeof(fdt64_t);
	else
		mem_reg += sizeof(fdt32_t);

	/* -1 because base is a legitimate memory address */
	pm_t top = (pm_t)fdt_load_int_ptr(ci.size_cells, mem_reg) + base - 1;
	return (struct pmem_layout){base, top};
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
	struct cell_info ci = get_reginfo(fdt, stdout);
	void *reg_ptr = (void *)fdt_getprop(fdt, stdout_offset, "reg", NULL);

	void *uart_ptr = 0;
	uart_ptr = (void *)(pm_t)fdt_load_int_ptr(ci.addr_cells, reg_ptr);

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
	extern char __init_end, __kernel_size;
	return (pm_t)&__init_end + (pm_t)&__kernel_size;
}

static pm_t get_initrdtop(void *fdt)
{
	int chosen_offset = fdt_path_offset(fdt, "/chosen");
	struct cell_info ci = get_cellinfo(fdt, chosen_offset);

	void *initrd_end_ptr = (void *)fdt_getprop(fdt, chosen_offset,
			"linux,initrd-end", NULL);

	return (pm_t)fdt_load_int_ptr(ci.addr_cells, initrd_end_ptr);
}

static pm_t get_initrdbase(void *fdt)
{
	int chosen_offset = fdt_path_offset(fdt, "/chosen");
	struct cell_info ci = get_cellinfo(fdt, chosen_offset);

	void *initrd_base_ptr = (void *)fdt_getprop(fdt, chosen_offset,
			"linux,initrd-start", NULL);

	return (pm_t)fdt_load_int_ptr(ci.addr_cells, initrd_base_ptr);
}

static pm_t get_fdttop(void *fdt)
{
	const char *b = (const char *)fdt;
	return (pm_t)(b + fdt_totalsize(fdt));
}

static pm_t get_fdtbase(void *fdt)
{
	/* lol */
	return (pm_t)fdt;
}

static void mark_area_used(pm_t base, pm_t top)
{
	size_t area_left = top - base;
	while(area_left >= MM_TPAGE_SIZE){
		mark_used(MM_TPAGE, base);
		area_left -= MM_TPAGE_SIZE;
		base += MM_TPAGE_SIZE;
	}

	while(area_left >= MM_GPAGE_SIZE){
		mark_used(MM_GPAGE, base);
		area_left -= MM_GPAGE_SIZE;
		base += MM_GPAGE_SIZE;
	}

	while(area_left >= MM_MPAGE_SIZE){
		mark_used(MM_MPAGE, base);
		area_left -= MM_MPAGE_SIZE;
		base += MM_MPAGE_SIZE;
	}


	while(area_left >= MM_KPAGE_SIZE){
		mark_used(base, MM_KPAGE);
		area_left -= MM_KPAGE_SIZE;
		base += MM_KPAGE_SIZE;
	}

	if(area_left != 0)
		mark_used(base, MM_KPAGE_SIZE);
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

	/* TODO: check that pmap placement doesn't overwrite anything, such as
	 * stack or go over top address of memory */
	size_t probe_size = probe_pmap(pmem.base, pmem.top - pmem.base);
	/* riscv handles two byte boundaries better than one byte, so align
	 * upwards */
	pm_t pmap_base = align_up(top + 1, 2);
	size_t actual_size = populate_pmap(pmem.base, pmem.top - pmem.base,
			pmap_base);

	/* TODO: not entirely sure what to do about this, probably give up trying to
	 * boot? */
	if(probe_size != actual_size){
		dbg("BUG! probe_size (%#lx) != actual_size (%#lx)\n",
				probe_size, actual_size);
	}

	/* mark init stack, at the moment always mapped to 2M */
	mark_used(PM_STACK_BASE, MM_MPAGE);

	/* mark kernel, at the moment it is always mapped to a 2M partition */
	mark_used(PM_KERN, MM_MPAGE);

	/* mark fdt and initrd */
	mark_area_used(get_initrdbase(fdt), initrd_top);
	mark_area_used(get_fdtbase(fdt), fdt_top);

	/* mark pmap */
	mark_area_used(pmap_base, pmap_base + actual_size);
}

void init(void *fdt)
{
	init_debug(fdt);
	dbg_fdt(fdt);
	setup_pmem(fdt);

	/* update_pmap(TODO: figure out where to place pmap in vmem); */
	/* TODO: jump to kernel */
}
