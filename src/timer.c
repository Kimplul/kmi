/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file timer.c
 * Timer handling implementation. Currently we only expect an architecture to
 * support a single timer per core.
 *
 * By keeping all timers in a binary search
 * tree ordered by time, we can just set the single timer to interrupt us when
 * the next timer is due and with thread info call the thread that set the
 * timer. From what I can tell, this is largely what Linux does.
 *
 * \todo Figure out if there are any advantages to having multiple concurrent
 * timers.
 */

#include <kmi/sp_tree.h>
#include <kmi/string.h>
#include <kmi/notify.h>
#include <kmi/nodes.h>
#include <kmi/utils.h>
#include <kmi/timer.h>
#include <kmi/debug.h>
#include <kmi/bkl.h>

#include <arch/timer.h>
#include <arch/cpu.h>

/** Timer resolution. */
static ticks_t ticks_per_sec = 0;

/** Array of timer maps for each cpu. */
static struct sp_root cpu_timers[MAX_CPUS] = { 0 };

/** Timer node subsystem instance. */
static struct node_root node_root;

/** Node in timer map. */
struct timer_node {
	/** Sp tree node. */
	struct sp_node sp_n;

	/** Corresponding timer. */
	struct timer timer;
};

/**
 * Get \ref timer_node from \ref sp_node.
 *
 * @param ptr \ref sp_node whose parent \ref timer_node to get.
 * @return Corresponding \ref timer_node.
 */
#define timer_container(ptr) container_of(ptr, struct timer_node, sp_n)

/**
 * Get \ref timer_node from \ref timer.
 *
 * @param ptr \ref timer whose parent \ref timer_node to get.
 * @return Corresponding \ref timer_node.
 */
#define timer_node_container(ptr) container_of(ptr, struct timer_node, timer)

/**
 * Get timer map of current cpu.
 *
 * @return Root of current cpu's timer map.
 */
static struct sp_root *__cpu_timers()
{
	return &cpu_timers[cpu_id()];
}

void init_timer(const void *fdt)
{
	ticks_per_sec = stat_timer(fdt);
	info("ticks_per_sec: %" PRIu64 "\n", ticks_per_sec);
	info("current ticks: %" PRIu64 "\n", current_ticks());
	init_nodes(&node_root, sizeof(struct timer_node));
}

/**
 * Insert timer into current cpu's timer map.
 *
 * \note \c ti.cid might change during the insertion if there already is a node
 * with identical \c cid to avoid collisions. Very unlikely though.
 *
 * @param ti Timer node to insert.
 * @return \c cid of timer node.
 */
static id_t __insert_timer(struct timer_node *ti)
{
	struct sp_root *root = __cpu_timers();
	struct sp_node *n = sp_root(root), *p = NULL;
	enum sp_dir d = SP_LEFT;
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
			 * occurs.
			 *
			 * This is one thing we couldn't do without having the
			 * internals of sptrees exposed. Pretty cool, huh? */
			ti->timer.cid++;
		}

		p = n;

		if (ti->timer.cid < t->timer.cid) {
			n = sp_left(n);
			d = SP_LEFT;
		} else {
			n = sp_right(n);
			d = SP_RIGHT;
		}
	}

	sp_insert(&sp_root(root), p, &ti->sp_n, d);
	return ti->timer.cid;
}

/**
 * Create timer at absolute timepoint.
 *
 * @param tid Requesting thread ID.
 * @param ticks Absolute timepoint.
 * @return \c cid of created timer.
 */
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

stat_t remove_timer(struct timer *t)
{
	if (!t)
		return ERR_INVAL;

	struct sp_node *n = &timer_node_container(t)->sp_n;
	sp_remove(&sp_root(__cpu_timers()), n);
	free_node(&node_root, t);
	return OK;
}

ticks_t nsecs_to_ticks(tunit_t nsecs)
{
	ticks_t t = (nsecs * ticks_per_sec) / 1000000000;
	return t == 0 ? 1 : t;
}

/* call to this function from exception handlers */
void handle_timer()
{
	/** @todo should this also disable irqs? */
	bkl_lock();
	struct timer *t = newest_timer();
	id_t tid = t->tid;
	remove_timer(t);

	struct tcb *r = get_tcb(tid);
	if (!r || orphan(r)) {
		info("tcb %llu dead at timer\n",
				(unsigned long long)tid);
		bkl_unlock();
		return;
	}

	notify(r, NOTIFY_TIMER);
	bkl_unlock();
}
