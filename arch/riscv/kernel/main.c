#include <apos/debug.h>
#include <apos/init.h>

void __main main(struct init_data_t d)
{
	/* TODO: approximate order of business:
	 * setup debugging in vmem
	 * free unnecessary init
	 * setup interrupts (should this be done in init?)
	 * load init from initrd
	 * setup init environment (thread control blocks etc.)
	 * jumpstart init (free stack init setup)
	 */
}
