#include <apos/power.h>
#include <apos/sizes.h>
#include <apos/uapi.h>

size_t __proc_stack_size = SZ_2M;
size_t __call_stack_size = SZ_2M;

SYSCALL_DEFINE2(conf)(vm_t param, vm_t val)
{
	UNUSED(param);
	UNUSED(val);
	/* no parameters supported atm */

	return OK;
}

SYSCALL_DEFINE1(poweroff)(vm_t type)
{
	switch (type) {
		case SHUTDOWN:
		case COLD_REBOOT:
		case WARM_REBOOT:
			return poweroff(type);
	};

	return ERR_INVAL;
}
