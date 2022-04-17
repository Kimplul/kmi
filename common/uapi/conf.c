#include <apos/power.h>
#include <apos/sizes.h>
#include <apos/uapi.h>

size_t __proc_stack_size = SZ_2M;
size_t __call_stack_size = SZ_2M;

vm_t sys_conf(vm_t param, vm_t val, vm_t u0, vm_t u1)
{
	UNUSED(u0);
	UNUSED(u1);
	UNUSED(param);
	UNUSED(val);

	/* no parameters supported atm */

	return OK;
}

vm_t sys_poweroff(vm_t type, vm_t u0, vm_t u1, vm_t u2)
{
	UNUSED(u0);
	UNUSED(u1);
	UNUSED(u2);
	switch (type) {
		case SHUTDOWN:
		case COLD_REBOOT:
		case WARM_REBOOT:
			return poweroff(type);
	};

	return ERR_INVAL;
}
