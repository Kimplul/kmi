/* TODO: cleanup :P */

#include <apos/init.h>
#include <apos/sizes.h>
#include <apos/types.h>
#include <apos/string.h>
#include <apos/debug.h>
#include <apos/pmem.h>
#include <apos/vmem.h>
#include <apos/utils.h>
#include <libfdt.h>
#include <pages.h>
#include <csr.h>

struct pm_layout_t {
	pm_t base;
	pm_t top;
};

struct pm_orders_t {
	size_t max_order;
	size_t bits[10];
	size_t page_shift;
};

static struct pm_layout_t get_memlayout(void *fdt)
{
	struct cell_info_t ci = get_reginfo(fdt, "/memory");
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
	return (struct pm_layout_t){base, top};
}

#ifdef DEBUG

static void init_debug(void *fdt)
{
	struct dbg_info_t dbg = dbg_from_fdt(fdt);
	dbg_init(dbg.dbg_ptr, dbg.dev);
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
	struct cell_info_t ci = get_cellinfo(fdt, chosen_offset);

	void *initrd_end_ptr = (void *)fdt_getprop(fdt, chosen_offset,
			"linux,initrd-end", NULL);

	return (pm_t)fdt_load_int_ptr(ci.addr_cells, initrd_end_ptr);
}

static pm_t get_initrdbase(void *fdt)
{
	int chosen_offset = fdt_path_offset(fdt, "/chosen");
	struct cell_info_t ci = get_cellinfo(fdt, chosen_offset);

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
	struct cell_info_t ci = get_reginfo(fdt, "/reserved-memory/mmode_resv0");
	uint8_t *rmem_reg = (uint8_t *)fdt_getprop(fdt, rmem_offset, "reg", NULL);

	pm_t base = (pm_t)fdt_load_int_ptr(ci.addr_cells, rmem_reg);

	if(ci.addr_cells == 2)
		rmem_reg += sizeof(fdt64_t);
	else
		rmem_reg += sizeof(fdt32_t);

	pm_t top = (pm_t)fdt_load_int_ptr(ci.size_cells, rmem_reg) + base - 1;
	mark_area_used(base, top);
}

static struct pm_orders_t init_pmem(void *fdt)
{
	enum mm_mode_t mmode = get_mmode(fdt);

	size_t max_order = 0;
	size_t order_bits = 9;
	switch(mmode){
		case Sv32:
			max_order = 1;
			order_bits = 10;
			break;

		case Sv39:
			max_order = 2;
			break;

		case Sv48:
			max_order = 3;
			break;
	};

	size_t bits[10] = {0};
	for(size_t i = 0; i <= max_order; ++i)
		bits[i] = order_bits;

	init_mem(max_order, bits, 12);

	struct pm_orders_t ret = {max_order, {0}, 12};
	for(size_t i = 0; i <= __mm_max_order; ++i)
		ret.bits[i] = bits[i];

	return ret;
}

static struct pm_layout_t setup_pmem(void *fdt)
{
	struct pm_layout_t pmem = get_memlayout(fdt);

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

	/* mark reserved mem */
	mark_reserved_mem(fdt);

	return (struct pm_layout_t){.base = pmap_base, .top = actual_size + pmap_base};
}

pm_t move_kernel()
{
	extern char __init_end, __kernel_size;
	pm_t dst = alloc_page(MM_MPAGE, 0);
	memmove((void *)dst, &__init_end, (size_t)&__kernel_size);

	return dst;
}

struct vm_branch_t *prepare_vmem()
{
	pm_t kernel_dst = move_kernel();

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

	/* map root pte */
	map_vmem(branch, (pm_t)branch, ROOT_PTE, VM_R | VM_W | VM_V, MM_KPAGE);

	/* TODO: map more stuff? */
	return branch;
}

static vm_t prepare_tmp_pte(struct vm_branch_t *branch)
{
	/* make sure the TMP_PTE is mapped to *something* */
	map_vmem(branch, 0, TMP_PTE, VM_R | VM_W | VM_V, MM_KPAGE);

	struct vm_branch_t *tmp_pte = arch_get_tmp_pte(branch);

	vm_t addr = TMP_PTE + MM_KPAGE_SIZE;
	map_vmem(branch, (vm_t)tmp_pte, addr, VM_R | VM_W | VM_V, MM_KPAGE);
	return addr;
}

void start_vmem(void *fdt, struct vm_branch_t *branch)
{
	/* TODO: get ASID from CPU id */

	/* TODO: probably unnecessary optimisations but this could be cached? */
	enum mm_mode_t m = get_mmode(fdt);

	if(m == Sv32)
		csr_write(CSR_SATP, SATP_MODE_Sv32 | pm_to_pnum((pm_t)(branch)));
	else if (m == Sv39)
		csr_write(CSR_SATP, SATP_MODE_Sv39 | pm_to_pnum((pm_t)(branch)));
	else
		csr_write(CSR_SATP, SATP_MODE_Sv48 | pm_to_pnum((pm_t)(branch)));

	__asm__ ("sfence.vma" : : : "memory");
	/* Sv57 && Sv64 in the future? */
}

struct init_data_t populate_initdata(void *fdt, struct pm_orders_t o,
		struct pm_layout_t p,
		struct vm_branch_t *b,
		vm_t v)
{
	extern char __init_start, __init_end;

	struct init_data_t d = {0};
	d.init_base = (pm_t)&__init_start;
	d.init_top = (pm_t)&__init_end;

	d.initrd_base = get_initrdbase(fdt);
	d.initrd_top = get_initrdtop(fdt);

	d.pmap_base = p.base;
	d.pmap_top = p.top;

	d.fdt_base = get_fdtbase(fdt);
	d.fdt_top = get_fdttop(fdt);

	d.stack_base = PM_STACK_BASE;
	d.stack_top = PM_STACK_TOP;

	d.kernel_vm_base = b;
	d.tmp_pte = v;

	d.max_order = o.max_order;
	for(size_t i = 0; i <= __mm_max_order; ++i)
		d.bits[i] = o.bits[i];
	d.page_shift = o.page_shift;

	/* initialize pmap in vmem */
	pm_t addr = p.base;
	while(addr < p.top){
		map_vmem(b, addr, addr, VM_R | VM_W | VM_V, MM_MPAGE);
		addr += MM_MPAGE_SIZE;
	}

	return d;
}

void init(void *fdt)
{
	init_debug(fdt);
	dbg_fdt(fdt);

	struct pm_orders_t o = init_pmem(fdt);
	struct pm_layout_t p = setup_pmem(fdt);
	struct vm_branch_t *b = prepare_vmem();
	vm_t v = prepare_tmp_pte(b);
	struct init_data_t d = populate_initdata(fdt, o, p, b, v);
	start_vmem(fdt, b);

	/* update_pmap(TODO: figure out where to place pmap in vmem); */
	void (*main)(struct init_data_t) = (void (*)(struct init_data_t))VM_KERN;
	main(d);
}
