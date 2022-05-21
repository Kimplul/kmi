#include <arch/timer.h>
#include <libfdt.h>
#include <csr.h>
#include <sbi.h>

ticks_t stat_timer(const void *fdt)
{
	int cpu_offset = fdt_path_offset(fdt, "/cpus");
	uint8_t *tf_reg = (uint8_t *)fdt_getprop(fdt, cpu_offset,
	                                         "timebase-frequency", NULL);
	return (ticks_t)fdt_load_int32_ptr(tf_reg);
}

void set_timer(ticks_t ticks)
{
	sbi_set_timer(ticks);
}

#if __riscv_xlen == 64
static ticks_t get_ticks64()
{
	ticks_t ticks = 0;
	csr_read(CSR_TIME, ticks);
	return ticks;
}
#else
static ticks_t get_ticks64()
{
	ticks_t ticksh, ticksl, check;

	/* avoid overflow between reading high and low */
	do {
		csr_read(CSR_TIMEH, ticksh);
		csr_read(CSR_TIME, ticksl);
		csr_read(CSR_TIMEH, check);
	} while (ticksh != check);

	return (ticksh << 32) | ticksl;
}
#endif

ticks_t current_ticks()
{
	return get_ticks64();
}
