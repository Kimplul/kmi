/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2024, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_QUEUE_H
#define KMI_QUEUE_H

/**
 * @file queue.h
 *
 * Wrapper around lists to make things a bit more readable.
 */

#include <kmi/list.h>

/** A queue. */
struct queue_head {
	/** A queue is pretty much just a list. */
	struct list_head l;
};

/** Initialize static queue. */
#define INIT_QUEUE(name) {.l = INIT_LIST((name).l)}

/**
 * Push an entry to the back of the queue.
 * If \p new already exists in some queue, nothing is done.
 *
 * @param new Entry to push.
 * @param head Head of queue.
 */
static inline void queue_push(struct queue_head *new, struct queue_head *head)
{
	/* if new is already in list, don't change its relative positioning */
	if (in_list(&new->l))
		return;

	list_add_tail(&new->l, &head->l);
}

/**
 * Pop an entry from the front of the queue.
 *
 * @param head Head of queue.
 * @return Front of queue or NULL if empty.
 */
static inline struct queue_head *queue_pop(struct queue_head *head)
{
	struct list_head *e = list_front(&head->l);
	if (!e)
		return NULL;

	list_del(e);
	return container_of(e, struct queue_head, l);
}

/**
 * @param head Entry to check for queueing status.
 * @return \ref true if in queue, \ref false otherwise.
 */
static inline bool in_queue(struct queue_head *head)
{
	return in_list(&head->l);
}

/**
 * Remove entry from queue.
 *
 * @param head Entry to remove.
 */
static inline void queue_del(struct queue_head *head)
{
	if (in_queue(head))
		list_del(&head->l);
}

#endif /* QUEUE_H */
