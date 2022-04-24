#include <arch/timer.h>
#include <sbi.h>

ticks_t stat_timer(const void *fdt)
{
	/* TODO: read from fdt */
	return 0;
}

void set_timer(ticks_t ticks)
{
	/* TODO */
	sbi_set_timer(ticks);
}

ticks_t current_ticks()
{
	/* TODO */
	return 0;
}
