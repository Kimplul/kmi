#include <apos/debug.h>
#include <apos/vmem.h>
#include <apos/pmem.h>
#include <apos/utils.h>
#include <apos/types.h>
#include <apos/mem.h>
#include <apos/lock.h>
#include <apos/tcb.h>
#include <apos/string.h>
#include <apos/proc.h>
#include <apos/mem_nodes.h>
#include <apos/elf.h>
#include <apos/initrd.h>
#include <libfdt.h>
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

#ifdef DEBUG

static void init_dbg(void *fdt)
{
	struct dbg_info_t dbg = dbg_from_fdt(fdt);
	dbg_init(dbg.dbg_ptr, dbg.dev);
}

#else
#define init_dbg(...)
#endif

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
	return (struct pm_layout_t){(size_t)__va(base), (size_t)__va(top)};
}

static pm_t get_kerneltop()
{
	/* interesting, for some reason if I define these to be just char
	 * pointers I get some wacky values. Not sure why that would be, but
	 * this works. */
	extern char __kernel_end;
	return (pm_t)&__kernel_end;
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

	/* mark kernel, at the moment with a 2M partition */
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

static struct pm_orders_t init_pmem(void *fdt)
{
	/* bad idea for now */
	/* enum mm_mode_t mmode = get_mmode(fdt); */

	size_t max_order = 0;
	size_t order_bits = 9;
	enum mm_mode_t mmode = Sv39;
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

/* assume Sv39 for now */
#define to_pte(a, f) (((a) >> 12) << 10 | (f))
static void populate_root_branch(struct vm_branch_t *b)
{
	size_t flags = VM_V | VM_R | VM_W | VM_X | VM_G;
	for(size_t i = 256; i < 512; ++i)
		b->leaf[i] = (struct vm_branch_t *)to_pte(RAM_BASE + SZ_1G * (i - 256), flags);
}

static void start_vmem(struct vm_branch_t *branch, enum mm_mode_t m)
{
	/* TODO: get ASID from CPU id */

	branch = (struct vm_branch_t *)__pa(branch);

	if(m == Sv32)
		csr_write(CSR_SATP, SATP_MODE_Sv32 | pm_to_pnum((pm_t)(branch)));
	else if (m == Sv39)
		csr_write(CSR_SATP, SATP_MODE_Sv39 | pm_to_pnum((pm_t)(branch)));
	else
		csr_write(CSR_SATP, SATP_MODE_Sv48 | pm_to_pnum((pm_t)(branch)));

	__asm__ ("sfence.vma" : : : "memory");
	/* Sv57 && Sv64 in the future? */
}

static struct vm_branch_t *init_vmem(void *fdt)
{
	init_pmem(fdt);
	setup_pmem(fdt);

	struct vm_branch_t *b = (struct vm_branch_t *)alloc_page(MM_KPAGE, 0);
	memset(b, 0, sizeof(struct vm_branch_t));

	populate_root_branch(b);
	/* update which memory branch to use */
	start_vmem(b, Sv39);
	return b;
}

static void init_irq(void *fdt)
{
}

static void init_proc(void *fdt, struct vm_branch_t *b)
{
	struct tcb *t = (struct tcb *)alloc_page(MM_KPAGE, 0);
	t->b_r = b;
	t->pid = 0;
	t->tid = 0;
	threads_insert(t);
	/* first page reserved to avoid issues with null pointers being legal*/
	sp_mem_init(&t->sp_r, SZ_4K, SZ_256G);

	/* binary itself */
	size_t sz = get_init_size(fdt);
	/* stack (?) */
	t->stack = alloc_uvmem(t, SZ_2M, VM_V | VM_R | VM_W | VM_U);

	/* should probably wrap this in like tlb_flush_all() or something */
	__asm__ ("sfence.vma" : : : "memory");

	jump_to_userspace(t, get_init_base(fdt), 0, 0);
}

#define __va_reg(reg)\
{\
	vm_t reg = 0;\
	__asm__("mv %0, " QUOTE(reg) : "=r" (reg) :: );\
	reg = (vm_t)__va(reg);\
	__asm__("mv " QUOTE(reg) ", %0" :: "rK" (reg) : );\
}

void __main main(void *fdt)
{
	__va_reg(sp);
	__va_reg(fp);
	__va_reg(gp);

	/* correct fdt pointer */
	fdt = (void *)__va(fdt);
	init_dbg(fdt);
	dbg_fdt(fdt);

	/* allow supervisor code to touch user data */
	csr_set(CSR_SSTATUS, SSTATUS_SUM);
	/* maybe this is too early but mark that we want to jump to user mode
	 * eventually*/
	csr_clear(CSR_SSTATUS, SSTATUS_SPP);
	struct vm_branch_t *b = init_vmem(fdt);
	init_mem_blocks();
	init_irq(fdt);
	init_proc(fdt, b);
}
