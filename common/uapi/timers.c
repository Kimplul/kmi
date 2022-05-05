#include <apos/timer.h>
#include <apos/uapi.h>

static ticks_t scaled_ticks(vm_t ticks, vm_t repeat)
{
#if __WORDSIZE == 64
	UNUSED(repeat);
	return ticks;
#else
	return ((ticks_t)ticks << 32) + repeat;
#endif
}

SYSCALL_DEFINE0(timebase)()
{
	return secs_to_ticks(1);
}

SYSCALL_DEFINE2(req_rel_timer)(vm_t ticks, vm_t repeat)
{
	return new_rel_timer(cur_tcb()->tid, scaled_ticks(ticks, repeat));
}

SYSCALL_DEFINE2(req_abs_timer)(vm_t ticks, vm_t repeat)
{
	return new_abs_timer(cur_tcb()->tid, scaled_ticks(ticks, repeat));
}

SYSCALL_DEFINE1(free_timer)(vm_t cid)
{
	struct timer *timer = find_timer(cid);
	if (!timer)
		return ERR_NF;

	remove_timer(timer);
	return OK;
}
