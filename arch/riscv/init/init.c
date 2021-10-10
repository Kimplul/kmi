/* TODO: cleanup :P */

#include <apos/init.h>
#include <apos/sizes.h>
#include <apos/types.h>
#include <apos/string.h>
#include <apos/debug.h>
#include <apos/pmem.h>
#include <apos/utils.h>
#include <libfdt.h>
#include <pages.h>
#include <vmem.h>
#include <csr.h>

struct pmem_layout {
	paddr_t base;
	paddr_t top;
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
	paddr_t base = (paddr_t)fdt_load_int_ptr(ci.addr_cells, mem_reg);

	if(ci.addr_cells == 2)
		mem_reg += sizeof(fdt64_t);
	else
		mem_reg += sizeof(fdt32_t);

	/* -1 because base is a legitimate memory address */
	paddr_t top = (paddr_t)fdt_load_int_ptr(ci.size_cells, mem_reg) + base - 1;
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

	void *uart_ptr = (void *)(paddr_t)fdt_load_int_ptr(ci.addr_cells, reg_ptr);

	dbg_init(uart_ptr, dev);
}

#else
#define init_debug(...)
#endif

static paddr_t get_kerneltop()
{
	/* interesting, for some reason if I define these to be just char
	 * pointers I get some wacky values. Not sure why that would be, but
	 * this works. */
	extern char __init_end, __kernel_size;
	return (paddr_t)&__init_end + (paddr_t)&__kernel_size;
}

static paddr_t get_initrdtop(void *fdt)
{
	int chosen_offset = fdt_path_offset(fdt, "/chosen");
	struct cell_info ci = get_cellinfo(fdt, chosen_offset);

	void *initrd_end_ptr = (void *)fdt_getprop(fdt, chosen_offset,
			"linux,initrd-end", NULL);

	return (paddr_t)fdt_load_int_ptr(ci.addr_cells, initrd_end_ptr);
}

static paddr_t get_initrdbase(void *fdt)
{
	int chosen_offset = fdt_path_offset(fdt, "/chosen");
	struct cell_info ci = get_cellinfo(fdt, chosen_offset);

	void *initrd_base_ptr = (void *)fdt_getprop(fdt, chosen_offset,
			"linux,initrd-start", NULL);

	return (paddr_t)fdt_load_int_ptr(ci.addr_cells, initrd_base_ptr);
}

static paddr_t get_fdttop(void *fdt)
{
	const char *b = (const char *)fdt;
	return (paddr_t)(b + fdt_totalsize(fdt));
}

static paddr_t get_fdtbase(void *fdt)
{
	/* lol */
	return (paddr_t)fdt;
}

static void mark_area_used(paddr_t base, paddr_t top)
{
	size_t area_left = top - base;
	/* TODO: add in a method to make sure that we use as large mappings as
	 * possible. */
	while(area_left >= MM_KPAGE_SIZE){
		mark_used(base, MM_KPAGE);
		area_left -= MM_KPAGE_SIZE;
		base += MM_KPAGE_SIZE;
	}

	if(area_left != 0)
		mark_used(base, MM_KPAGE_SIZE);
}

static void mark_reserved_mem(void *fdt)
{
	int rmem_offset = fdt_path_offset(fdt, "/reserved-memory/mmode_resv0");
	struct cell_info ci = get_reginfo(fdt, "/reserved-memory/mmode_resv0");
	uint8_t *rmem_reg = (uint8_t *)fdt_getprop(fdt, rmem_offset, "reg", NULL);

	paddr_t base = (paddr_t)fdt_load_int_ptr(ci.addr_cells, rmem_reg);

	if(ci.addr_cells == 2)
		rmem_reg += sizeof(fdt64_t);
	else
		rmem_reg += sizeof(fdt32_t);

	paddr_t top = (paddr_t)fdt_load_int_ptr(ci.size_cells, rmem_reg) + base - 1;
	mark_area_used(base, top);
}

static void setup_pmem(void *fdt)
{
	struct pmem_layout pmem = get_memlayout(fdt);

	paddr_t initrd_top = get_initrdtop(fdt);
	paddr_t kernel_top = get_kerneltop();
	paddr_t fdt_top = get_fdttop(fdt);

	paddr_t top = MAX3(kernel_top, initrd_top, fdt_top);
	dbg("initrd_top:\t%#lx\n", initrd_top);
	dbg("kernel_top:\t%#lx\n", kernel_top);
	dbg("fdt_top:\t%#lx\n", fdt_top);

	/* TODO: check that pmap placement doesn't overwrite anything, such as
	 * stack or go over top address of memory */
	size_t probe_size = probe_pmap(pmem.base, pmem.top - pmem.base);
	/* riscv handles two byte boundaries better than one byte, so align
	 * upwards */
	paddr_t pmap_base = align_up(top + 1, 2);
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

	/* mark reserved mem */
	mark_reserved_mem(fdt);
}

paddr_t move_kernel()
{
	extern char __init_end, __kernel_size;
	paddr_t dst = alloc_page(MM_MPAGE, 0);
	memmove((void *)dst, &__init_end, (size_t)&__kernel_size);

	return dst;
}

struct vm_branch_t *prepare_vmem()
{
	paddr_t kernel_dst = move_kernel();

	/* TODO: check if this actually works */
	struct vm_branch_t *branch = (struct vm_branch_t *)alloc_page(MM_KPAGE, 0);
	memset(branch, 0, sizeof(struct vm_branch_t));

	/* TODO: check mapping flags, also iron out possible bugs etc in
	 * map_vmem */
	/* map kernel */
	map_vmem(branch, kernel_dst, VM_KERN, VM_R | VM_W | VM_G | VM_X | VM_V, MM_MPAGE);

	/* map init */
	map_vmem(branch, PM_KERN, PM_KERN, VM_R | VM_W | VM_X | VM_V, MM_MPAGE);

	/* map stack */
	map_vmem(branch, PM_STACK_BASE, PM_STACK_BASE, VM_R | VM_W | VM_V, MM_MPAGE);

	/* TODO: map more stuff? */
	return branch;
}

void start_vmem(struct vm_branch_t *branch)
{
	/* assume Sv48 and ASID 0*/
	/* TODO: get ASID from CPU id
	 * TODO: more cores lol
	 */
	csr_write(CSR_SATP, SATP_MODE_48 | (((paddr_t)(branch)) >> 12));
}

struct init_data_t populate_initdata(void *fdt, struct vm_branch_t *branch)
{
	extern char __init_start, __init_end;

	struct init_data_t d = {0};
	d.init_base = (paddr_t)&__init_start;
	d.init_top = (paddr_t)&__init_end;

	d.initrd_base = get_initrdbase(fdt);
	d.initrd_top = get_initrdtop(fdt);

	d.fdt_base = get_fdtbase(fdt);
	d.fdt_top = get_fdttop(fdt);

	d.stack_base = PM_STACK_BASE;
	d.stack_top = PM_STACK_TOP;

	d.kernel_vm_base = branch;

	return d;
}

void init(void *fdt)
{
	init_debug(fdt);
	dbg_fdt(fdt);
	setup_pmem(fdt);


	struct vm_branch_t *branch = prepare_vmem();
	struct init_data_t d = populate_initdata(fdt, branch);
	start_vmem(branch);

	/* update_pmap(TODO: figure out where to place pmap in vmem); */
	void (*main)(struct init_data_t) = (void (*)(struct init_data_t))VM_KERN;
	main(d);
}
