#include <apos/mem_nodes.h>
#include <apos/timer.h>
#include <apos/attrs.h>
#include <apos/proc.h>
#include <apos/debug.h>
#include <apos/vmem.h>
#include <arch/arch.h>
#include <arch/irq.h>
#include <libfdt.h>

void __main main(void *fdt)
{
	/* dbg uses direct mapping at this point */
	init_dbg(fdt);
	setup_dmap_dbg();
	dbg_fdt(fdt);

	setup_arch(fdt);

	init_pmem(fdt);
	struct vmem *b = init_vmem(fdt);

	/* start up debugging in kernel IO */
	setup_io_dbg(b);

	init_irq(fdt);
	init_timer(fdt);
	init_proc(fdt, b);
}
