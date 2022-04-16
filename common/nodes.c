#include <apos/mem.h>
#include <apos/pmem.h>
#include <apos/nodes.h>
#include <apos/string.h>

#define MAX_NODES(node_size)\
	((BASE_PAGE_SIZE - sizeof(struct node_region)) / (sizeof(enum node_status) + node_size))

#define region_to_nodes(r)\
	((enum node_status *)((char *)(r) + sizeof(struct node_region)))

#define node_status(r)\
	((enum node_status *)((char *)(r) - sizeof(enum node_status)))

#define node_region(r)\
	((struct node_region *)((uintptr_t)(r) & ~(BASE_PAGE_SIZE - 1)))

static struct node_region *__create_region()
{
	struct node_region *r = (struct node_region *)alloc_page(BASE_PAGE, 0);
	memset(r, FREE, BASE_PAGE_SIZE);
	return r;
}

void init_nodes(struct node_root *r, size_t node_size)
{
	r->head = __create_region();
	r->av_head = r->head;
	r->node_size = node_size;
}

void destroy_nodes(struct node_root *r)
{
	struct node_region *nr = r->head;
	while (nr) {
		struct node_region *d = nr;
		nr = nr->prev;
		free_page(BASE_PAGE, (pm_t)d);
	}
}

static void *__find_free_node(struct node_root *r, struct node_region *nr)
{
	enum node_status *w = region_to_nodes(nr);
	for (size_t i = 0; i < MAX_NODES(r->node_size); ++i) {
		if (*w != FREE) {
			w = (enum node_status *)(r->node_size + (uint8_t *)(w + 1));
			continue;
		}

		*w = USED;
		return (void *)(w + 1);
	}

	return 0;
}

static void __pop_av_head(struct node_root *r)
{
	struct node_region *t = r->av_head;
	r->av_head = r->av_head->next;
	if (r->av_head)
		r->av_head->av_prev = 0;

	t->av_next = 0;
	t->av_prev = 0;
}

void *get_node(struct node_root *r)
{
	if (!r)
		return 0;

	if (!r->av_head) {
		r->av_head = __create_region();

		r->av_head->prev = r->head;
		r->head->next = r->av_head;

		r->head = r->av_head;
	}

	void *p = __find_free_node(r, r->av_head);
	if (++r->av_head->used_nodes == MAX_NODES(r->node_size))
		__pop_av_head(r);

	return p;
}

static void __push_av_head(struct node_root *r, struct node_region *nr)
{
	nr->av_prev = 0;
	nr->av_next = r->av_head;
	if (r->av_head)
		r->av_head->av_prev = nr;

	r->av_head = nr;
}

static void __free_region(struct node_root *r, struct node_region *nr)
{
	struct node_region *av_n = nr->av_next;
	struct node_region *av_p = nr->av_prev;

	if (av_n)
		av_n->av_prev = av_p;

	if (av_p)
		av_p->av_next = av_n;

	if (nr == r->av_head)
		__pop_av_head(r);

	struct node_region *n = nr->next;
	struct node_region *p = nr->prev;

	if (n)
		n->prev = p;

	if (p)
		p->next = n;

	if (nr == r->head) {
		if (r->head->prev) {
			r->head->next = 0;
			r->head = r->head->prev;
		} else
			return;
	}

	free_page(BASE_PAGE, (pm_t)nr);
}

void free_node(struct node_root *r, void *p)
{
	enum node_status *w = node_status(p);
	*w = FREE;

	struct node_region *nr = node_region(w);

	if (--nr->used_nodes == 0) {
		__free_region(r, nr);
		return;
	}

	else if (!nr->av_next && !nr->av_prev)
		__push_av_head(r, nr);
}
