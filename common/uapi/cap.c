#include <apos/uapi.h>
#include <apos/caps.h>
#include <apos/tcb.h>

/**
 * @file cap.c
 * Sycalls handlers for thread capabilities.
 */

/**
 * Check that values are legal for thread ID and offset.
 *
 * @param tid Thread ID of thread whose caps should be fetched.
 * @param off Offset of capabilities.
 * @return Pointer to capabilities of \p tid, \c 0 otherwise.
 */
static capflags_t *__get_tcb_caps(id_t tid, size_t off)
{
	if (!cap_off_ok(off))
		return NULL;

	struct tcb *t = get_tcb(tid);
	if (!t)
		return NULL;

	return &t->caps;
}

/**
 * Set capabilities.
 *
 * @param tid Thread ID whose capabilities to set.
 * @param off Offset of capability, multiple of \c bits(cap).
 * @param caps Mask of capabilities to set.
 * @return \ref OK on success, \ref ERR_INVAL on invalid input.
 */
SYSCALL_DEFINE3(set_cap)(sys_arg_t tid, sys_arg_t off, sys_arg_t caps)
{
	struct tcb *t = cur_tcb();
	if (!is_set(t->caps, CAP_CAPS))
		return (struct sys_ret){ERR_PERM, 0, 0, 0, 0, 0};

	capflags_t *c;
	if (!(c = __get_tcb_caps(tid, off)))
		return (struct sys_ret){ERR_INVAL, 0, 0, 0, 0, 0};

	set_caps(*c, off, caps);
	return (struct sys_ret){OK, 0, 0, 0, 0, 0};
}

/**
 * Get capabilities.
 *
 * @param tid Thread ID whose capabilities to get.
 * @param off Offset of capability, multiple of \c bits(cap).
 * @return \ref OK, capabilities.
 */
SYSCALL_DEFINE2(get_cap)(sys_arg_t tid, sys_arg_t off)
{
	capflags_t *c;
	if (!(c = __get_tcb_caps(tid, off)))
		return (struct sys_ret){ERR_INVAL, 0, 0, 0, 0, 0};

	return (struct sys_ret){OK, get_caps(*c, off), 0, 0, 0, 0};
}

/**
 * Clear capabilities.
 *
 * @param tid Thread ID whose capabilities to clear.
 * @param off Offset of capability, multiple of \c bits(cap).
 * @param caps Mask of capabilities to clear.
 * @return \ref OK.
 */
SYSCALL_DEFINE3(clear_cap)(sys_arg_t tid, sys_arg_t off, sys_arg_t caps)
{
	struct tcb *t = cur_tcb();
	if (!is_set(t->caps, CAP_CAPS))
		return (struct sys_ret){ERR_PERM, 0, 0, 0, 0, 0};

	capflags_t *c;
	if (!(c = __get_tcb_caps(tid, off)))
		return (struct sys_ret){ERR_INVAL, 0, 0, 0, 0, 0};

	clear_caps(*c, off, caps);
	return (struct sys_ret){OK, 0, 0, 0, 0, 0};
}
