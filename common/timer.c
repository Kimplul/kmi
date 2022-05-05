#include <apos/sp_tree.h>
#include <apos/string.h>
#include <apos/nodes.h>
#include <apos/utils.h>
#include <apos/timer.h>
#include <arch/timer.h>
#include <arch/cpu.h>

static ticks_t ticks_per_sec = 0;
static struct sp_root cpu_timers[MAX_CPUS] = { 0 };
static struct node_root node_root;

struct timer_node {
	struct sp_node sp_n;
	struct timer timer;
};

#define timer_container(ptr)      container_of(ptr, struct timer_node, sp_n)

#define timer_node_container(ptr) container_of(ptr, struct timer_node, timer)

static struct sp_root *__cpu_timers()
{
	return &cpu_timers[cpu_id()];
}

void init_timer(const void *fdt)
{
	ticks_per_sec = stat_timer(fdt);
	init_nodes(&node_root, sizeof(struct timer_node));
}

static id_t __insert_timer(struct timer_node *ti)
{
	struct sp_root *root = __cpu_timers();
	struct sp_node *n = sp_root(root), *p = NULL;
	enum sp_dir d;
	while (n) {
		struct timer_node *t = container_of(n, struct timer_node, sp_n);
		if (ti->timer.cid == t->timer.cid) {
			/* if there's an identical ID, we'll just increment our
			 * ID until we get and ID that doesn't exist yet. There
			 * is a very small possibility that this will set a
			 * timer that's very slightly ahead of some other timer
			 * to be handled after the one that's very close, but
			 * the timescales that we're dealing with are probably
			 * tiny enough that this won't matter, even if it
			 * occurs. */
			ti->timer.cid++;
		}

		p = n;

		if (ti->timer.cid < t->timer.cid) {
			n = sp_left(n);
			d = LEFT;
		} else {
			n = sp_right(n);
			d = RIGHT;
		}
	}

	if (sp_root(root))
		sp_insert(&sp_root(root), p, &ti->sp_n, d);
	else
		sp_root(root) = &ti->sp_n;

	return ti->timer.cid;
}

/* ticks is absolute */
static id_t __new_timer(id_t tid, ticks_t ticks)
{
	struct timer_node *ti = (struct timer_node *)get_node(&node_root);
	ti->timer.ticks = ticks;
	/* preliminary ID, may change after actual insertion */
	ti->timer.cid = ticks;
	ti->timer.tid = tid;
	return __insert_timer(ti);
}

/* these are likely not perfectly accurate timers due to some random delay from
 * function calls etc, but probably good enough. */
id_t new_rel_timer(id_t tid, ticks_t ticks)
{
	return new_abs_timer(tid, ticks + current_ticks());
}

id_t new_abs_timer(id_t tid, ticks_t ticks)
{
	id_t id = __new_timer(tid, ticks);
	set_timer(ticks);
	return id;
}

struct timer *newest_timer()
{
	struct sp_node *t = sp_first(sp_root(__cpu_timers()));
	return &timer_container(t)->timer;
}

struct timer *find_timer(id_t cid)
{
	struct sp_node *n = sp_root(__cpu_timers());
	while (n) {
		struct timer_node *t = timer_container(n);
		if (t->timer.cid == cid)
			return &t->timer;

		if (t->timer.cid < cid)
			n = sp_left(n);
		else
			n = sp_right(n);
	}

	return 0;
}

void remove_timer(struct timer *t)
{
	if (!t)
		return;

	struct sp_node *n = &timer_node_container(t)->sp_n;
	sp_remove(&sp_root(__cpu_timers()), n);
}

ticks_t nsecs_to_ticks(tunit_t nsecs)
{
	ticks_t t = (nsecs * ticks_per_sec) / 1000000000;
	return t == 0 ? 1 : t;
}

/* call to this function from exception handlers */
void update_timers()
{
	struct timer *t = newest_timer();
	remove_timer(t);

	/* TODO: handle timer thread ID */
}
