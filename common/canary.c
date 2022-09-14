#include <apos/canary.h>
#include <apos/mem.h>

/**
 * @file canary.c
 * Kernel stack canary implementation.
 */

/** Typedef for canary value type. */
typedef uint32_t canary_t;

/** Canary magic value. */
static const canary_t canary = 0xb00b1e5;

/**
 * Helper for calculating the canary location of \p t.
 *
 * @param t \ref tcb to calculate canary position of.
 * @return Pointer to canary location, that is bottom of stack.
 * \todo This assumes that all stacks grow downwards. Unlikely to ever be
 * ported to a platform that doesn't abide by this, but keep it in mind anyway.
 */
static canary_t *get_canary(struct tcb *t)
{
	size_t s = order_size(KERNEL_STACK_PAGE_ORDER);
	return (canary_t *)align_down((uintptr_t)t, s);
}

void set_canary(struct tcb *t)
{
	canary_t *c = get_canary(t);
	*c = canary;
}

bool check_canary(struct tcb *t)
{
	canary_t *c = get_canary(t);
	return *c != canary;
}
