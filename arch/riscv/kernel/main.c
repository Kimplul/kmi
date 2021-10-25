#include <apos/debug.h>
#include <apos/init.h>
#include <apos/vmem.h>
#include <apos/mem.h>

#ifdef DEBUG

static void kernel_dbg(void *fdt)
{
	struct dbg_info_t dbg = dbg_from_fdt(fdt);
	dbg_init(dbg.dbg_ptr, dbg.dev);
}

#else
#define kernel_dbg(...)
#endif

static void map_fdt(struct vm_branch_t *branch, vm_t fdt_base, vm_t fdt_top)
{
	map_vregion(branch, fdt_base, 0, fdt_top - fdt_base, VM_R | VM_W |VM_V);
}

void __main main(struct init_data_t d)
{
	/* feck */
	dbg_init(0x10000000, NS16550A);
	dbg("test\n");
	init_pmap((void *)d.pmap_base);
	init_mem(d.max_order, d.bits, d.page_shift);
	init_vmem(ROOT_PTE, d.tmp_pte);
	/* TODO: move pmap into vmem somewhere */
	/* TODO: mark kernel area as used */
	map_fdt(ROOT_PTE, d.fdt_base, d.fdt_top);
	kernel_dbg((void *)d.fdt_base);
	/* TODO: approximate order of business:
	 * setup debugging in vmem (requires mapping fdt)
	 * free unnecessary init
	 * setup interrupts (should this be done in init?)
	 * load init from initrd
	 * setup init environment (thread control blocks etc.)
	 * jumpstart init (free stack init setup)
	 */

	/* functionality that should be implemented:
	 * figure out best continuous run of memory (pmap etc)
	 * arbitrary user and kernel mapping (memory)
	 * vm to pm
	 */
}
