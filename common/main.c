#include <apos/mem_nodes.h>
#include <apos/attrs.h>
#include <apos/proc.h>
#include <apos/debug.h>
#include <apos/vmem.h>
#include <apos/arch.h>
#include <apos/irq.h>
#include <libfdt.h>

#ifdef DEBUG
static struct dbg_info dbg_info = (struct dbg_info){0};

static void init_dbg(void *fdt)
{
	dbg_info = dbg_from_fdt(fdt);
}

static void setup_dmap_dbg()
{
	setup_dbg(dbg_info.dbg_ptr, dbg_info.dev);
}

static void setup_io_dbg(struct vm_branch *b)
{
	vm_t io_ptr = setup_kernel_io(b, dbg_info.dbg_ptr);
	setup_dbg(io_ptr, dbg_info.dev);
}
#else
#define init_dbg(...)
#define setup_dmap_dbg(...)
#define setup_io_dbg(...)
#endif

void __main main(void *fdt)
{
	/* dbg uses direct mapping at this point */
	init_dbg(fdt);
	setup_dmap_dbg();
	dbg_fdt(fdt);

	arch_setup(fdt);

	init_pmem(fdt);
	struct vm_branch *b = init_vmem(fdt);

	/* start up debugging in kernel IO */
	setup_io_dbg(b);

	init_irq(fdt);
	init_proc(fdt, b);
}
