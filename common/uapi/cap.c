#include <kmi/uapi.h>
#include <kmi/caps.h>
#include <kmi/tcb.h>

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
 * @param t Current tcb.
 * @param tid Thread ID whose capabilities to set.
 * @param off Offset of capability, multiple of \c bits(cap).
 * @param caps Mask of capabilities to set.
 * @return \ref OK on success, \ref ERR_INVAL on invalid input.
 */
SYSCALL_DEFINE3(set_cap)(struct tcb *t, sys_arg_t tid, sys_arg_t off,
                         sys_arg_t caps)
{
	if (!is_set(t->caps, CAP_CAPS))
		return_args(t, SYS_RET1(ERR_PERM));

	capflags_t *c;
	if (!(c = __get_tcb_caps(tid, off)))
		return_args(t, SYS_RET1(ERR_INVAL));

	set_caps(*c, off, caps);
	return_args(t, SYS_RET1(OK));
}

/**
 * Get capabilities.
 *
 * @param t Current tcb.
 * @param tid Thread ID whose capabilities to get.
 * @param off Offset of capability, multiple of \c bits(cap).
 * @return \ref OK, capabilities.
 */
SYSCALL_DEFINE2(get_cap)(struct tcb *t, sys_arg_t tid, sys_arg_t off)
{
	capflags_t *c;
	if (!(c = __get_tcb_caps(tid, off)))
		return_args(t, SYS_RET1(ERR_INVAL));

	return_args(t, SYS_RET2(OK, get_caps(*c, off)));
}

/**
 * Clear capabilities.
 *
 * @param t Current tcb.
 * @param tid Thread ID whose capabilities to clear.
 * @param off Offset of capability, multiple of \c bits(cap).
 * @param caps Mask of capabilities to clear.
 * @return ERR_LERM if invalid permissions, ERR_INVAL if \p tid doesn't exist,
 * otherwise OK.
 */
SYSCALL_DEFINE3(clear_cap)(struct tcb *t, sys_arg_t tid, sys_arg_t off,
                           sys_arg_t caps)
{
	if (!is_set(t->caps, CAP_CAPS))
		return_args(t, SYS_RET1(ERR_PERM));

	capflags_t *c;
	if (!(c = __get_tcb_caps(tid, off)))
		return_args(t, SYS_RET1(ERR_INVAL));

	clear_caps(*c, off, caps);
	return_args(t, SYS_RET1(OK));
}
