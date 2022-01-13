#include <apos/vmem.h>
#include <apos/pmem.h>
#include <apos/mem.h>
#include <apos/string.h>
#include <apos/mem_nodes.h>

enum block_status {
	FREE = 0, USED = 1
};

struct block_wrapper {
	enum block_status status;
	struct mem_region n;
};

struct block_region {
	size_t used_blocks;

	struct block_region *av_next;
	struct block_region *av_prev;

	struct block_region *next;
	struct block_region *prev;
};

#define MAX_BLOCKS \
	((BASE_PAGE_SIZE - sizeof(struct block_region)) / sizeof(struct block_wrapper))

#define block_region(b) \
	((struct block_region *)((size_t)(b) & ~(BASE_PAGE_SIZE - 1)))

#define block_container(b) \
	container_of(b, struct block_wrapper, n)

#define region_to_array(r) \
	((struct block_wrapper *)((char *)(r) + sizeof(struct block_region)))

static struct block_region *head = 0;
static struct block_region *av_head = 0;

static struct block_region *__create_region()
{
	struct block_region *r = (struct block_region *)alloc_page(BASE_PAGE, 0);
	memset(r, FREE, BASE_PAGE_SIZE);
	return r;
}

void init_mem_blocks()
{
	head = __create_region();
	av_head = head;
}

void destroy_mem_blocks()
{
	struct block_region *r = head;
	while(r){
		struct block_region *d = r;
		r = r->prev;
		free_page(MM_O0, (pm_t)d);
	}
}

static struct mem_region *__find_free_block(struct block_region *h)
{
	struct block_wrapper *w = region_to_array(h);
	for(size_t i = 0; i < MAX_BLOCKS; ++i){
		if(w[i].status != FREE)
			continue;

		w[i].status = USED;
		return &w[i].n;
	}

	return 0;
}

static void __pop_av_head()
{
	struct block_region *t = av_head;
	av_head = av_head->av_next;
	if(av_head)
		av_head->av_prev = 0;

	t->av_next = 0;
	t->av_prev = 0;
}

struct mem_region *get_mem_node()
{
	if(!av_head){
		av_head = __create_region();

		av_head->prev = head;
		head->next = av_head;

		head = av_head;
	}

	struct mem_region *ret = __find_free_block(av_head);

	if(++av_head->used_blocks == MAX_BLOCKS)
		__pop_av_head();

	return ret;
}

static void __push_av_head(struct block_region *r)
{
	r->av_prev = 0;
	r->av_next = av_head;
	if(av_head)
		av_head->av_prev = r;

	av_head = r;
}

static void __free_block(struct block_region *r)
{
	struct block_region *av_n = r->av_next;
	struct block_region *av_p = r->av_prev;

	if(av_n)
		av_n->av_prev = av_p;

	if(av_p)
		av_p->av_next = av_n;

	if(r == av_head)
		__pop_av_head();

	struct block_region *n = r->next;
	struct block_region *p = r->prev;

	if(n)
		n->prev = p;

	if(p)
		p->next = n;

	if(r == head){
		if(head->prev){
			head->next = 0;
			head = head->prev;
		} else
			return;
	}

	free_page(BASE_PAGE, (pm_t)r);
}

void free_mem_node(struct mem_region *m)
{
	struct block_wrapper *w = block_container(m);
	w->status = FREE;

	struct block_region *r = block_region(w);

	if(--r->used_blocks == 0){
		__free_block(r);
		return;
	}

	else if(!r->av_next && !r->av_prev)
		__push_av_head(r);
}
