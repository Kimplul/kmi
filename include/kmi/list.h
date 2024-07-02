/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2024, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_LIST_H
#define KMI_LIST_H

/**
 * @file list.h
 *
 * Relatively simple intrusive doubly-linked list implementation,
 * more or less directly lifted from Linux. In Linux, the implementation also
 * includes some circular features that I dropped because I don't really need
 * them at the moment, but can just be copy-pasted back in (I think? Not a
 * license lawyer)
 *
 * https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/linux/list.h
 */

/** An entry in a list. Generally there's a head somewhere that is more like
 * just 'metadata' about the list. */
struct list_head {
	/** Previous entry. */
	struct list_head *next;

	/** Next entry. */
	struct list_head *prev;
};

/**
 * Initialized a static list.
 *
 * @param name Name of list we're initializing, important because an empty list
 * should just point to itself.
 */
#define INIT_LIST(name) {.next = &(name), .prev = &(name)}

/**
 * Add \p new between \p prev and \p next.
 *
 * @param new New entry to add to list.
 * @param prev Previous entry.
 * @param next Next entry.
 */
static inline void __list_add(struct list_head *new, struct list_head *prev,
                              struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * Add an entry to the front of the list.
 *
 * @param new Entry to add.
 * @param head Head of list.
 */
static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

/**
 * Add an entry to the back of the list.
 *
 * @param new Entry to add.
 * @param head Head of list.
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

/**
 * Link \p prev to be directly behind \p next and vice versa.
 *
 * @param prev Previous entry.
 * @param next Next entry.
 */
static inline void __list_link(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * Delete an entry from a list by linking the previous and next nodes together.
 *
 * @param entry Entry to delete from list. Whatever list that is, doesn't
 * matter.
 */
static inline void __list_del_entry(struct list_head *entry)
{
	__list_link(entry->next, entry->prev);
}

/**
 * Delete an entry from a list. Sets next and prev pointers to be NULL.
 *
 * @param entry Entry to delete from list.
 */
static inline void list_del(struct list_head *entry)
{
	__list_del_entry(entry);
	entry->next = NULL;
	entry->prev = NULL;
}

/**
 * Check if entry is part of some list.
 *
 * @param entry Entry whose list occupancy should be checked.
 * @return \ref true if entry is in some list, \ref false otherwise.
 */
static inline bool in_list(struct list_head *entry)
{
	return entry->next != NULL;
}

/**
 * Check if list is empty.
 *
 * @param head Head of list.
 * @return \ref true if list is empty, \ref false otherwise.
 */
static inline bool list_empty(struct list_head *head)
{
	return head->next == head;
}

/**
 * @param head Head of list.
 * @return First entry in list or NULL if empty.
 */
static inline struct list_head *list_front(struct list_head *head)
{
	if (list_empty(head))
		return NULL;

	return head->next;
}

/**
 * @param head Head of list.
 * @return First entry in list or NULL if empty.
 */
static inline struct list_head *list_back(struct list_head *head)
{
	if (list_empty(head))
		return NULL;

	return head->prev;
}

#endif /* KMI_LIST_H */
