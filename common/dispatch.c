#include <apos/uapi.h>
#include <apos/ipi.h>

/**
 * @file dispatch.c
 *
 * Interface between assembly and C for dispatching syscalls/IPIs.
 * In apos, IPIs are a higher level concept and is not expected to be handled by
 * the underlying architecture. Instead, we only require that there is some way
 * to trigger an interrupt in some other core, and check if the interrupt was an
 * IPI or syscall in C.
 */

/**
 * Syscall/IPI handler.
 * If a syscall was triggered, parameters have the meaning they are given.
 * Otherwise, they are meaningless and unused.
 *
 * @param a Syscall number.
 * @param b Argument 0.
 * @param c Argument 1.
 * @param d Argument 2.
 * @param e Argument 3.
 * @param f Argument 4.
 *
 * Returns value of taken action.
 */
void dispatch(sys_arg_t a, sys_arg_t b, sys_arg_t c,
              sys_arg_t d, sys_arg_t e, sys_arg_t f)
{
	struct tcb *t = cur_tcb();
	if (clear_ipi(t)) {
		handle_ipi(t);
		return;
	}

	handle_syscall(a, b, c, d, e, f, t);
}
