#ifndef APOS_TIMER_H
#define APOS_TIMER_H

#include <apos/types.h>

typedef size_t ticks_t;
/* whichever time unit we're dealing with */
typedef size_t tunit_t;

struct timer {
	id_t tid;
	id_t cid;

	ticks_t ticks;
};

void init_timer();

/* set up timer interrupt ticks from now */
id_t new_rel_timer(id_t tid, ticks_t ticks);
/* set up timer interrupt at ticks (from startup I suppose) */
id_t new_abs_timer(id_t tid, ticks_t ticks);

struct timer *newest_timer();
struct timer *find_timer(id_t cid);
void remove_timer(struct timer *);

ticks_t nsecs_to_ticks(tunit_t nsecs);

static inline ticks_t usecs_to_ticks(tunit_t usecs)
{
	return nsecs_to_ticks(usecs * 1000);
}

static inline ticks_t msecs_to_ticks(tunit_t msecs)
{
	return usecs_to_ticks(msecs * 1000);
}

/* TODO: likely not a problem on 64bit systems, not sure how to handle situation on
 * 32bit */
static inline ticks_t secs_to_ticks(tunit_t secs)
{
	return msecs_to_ticks(secs * 1000);
}

#endif /* APOS_TIMER_H */
