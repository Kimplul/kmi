#include <apos/uapi.h>
#include <apos/ipi.h>

struct sys_ret dispatch(sys_arg_t a, sys_arg_t b, sys_arg_t c,
                        sys_arg_t d, sys_arg_t e, sys_arg_t f)
{
	struct tcb *t = cur_tcb();
	if (clear_ipi(t))
		return handle_ipi(t);

	return handle_syscall(t, a, b, c, d, e, f);
}
