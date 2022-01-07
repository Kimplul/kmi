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
	struct sp_node sp_n;
};

static struct sp_root root_region = (struct sp_root){0};

#define MAX_BLOCKS \
	((BASE_PAGE_SIZE - sizeof(struct block_region)) / sizeof(struct block_wrapper))

#define block_region(b) \
	((struct block_region *)((size_t)(b) & ~(BASE_PAGE_SIZE - 1)))

#define region_container(b) \
	container_of(b, struct block_region, sp_n)

#define block_container(b) \
	container_of(b, struct block_wrapper, n)

#define region_to_array(r) \
	((struct block_wrapper *)((char *)(r) + sizeof(struct block_region)))

static struct block_region *__create_region()
{
	struct block_region *r = (struct block_region *)alloc_page(BASE_PAGE, 0);
	memset(r, FREE, BASE_PAGE_SIZE);
	return r;
}

void init_mem_blocks()
{
	sp_root(root_region) = &__create_region()->sp_n;
}

static void __destroy_mem_block(struct sp_node *n)
{
	if(!n)
		return;

	__destroy_mem_block(sp_left(n));
	__destroy_mem_block(sp_right(n));

	struct block_region *r = block_region(n);
	free_page(BASE_PAGE, (vm_t)r);
}

void destroy_mem_blocks()
{
	__destroy_mem_block(sp_root(root_region));
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

static void __region_insert(struct block_region *r)
{
	struct sp_node *n = sp_root(root_region), *p = NULL;
	enum sp_dir d = LEFT;

	r->sp_n = (struct sp_node){0};

	while(n){
		struct block_region *t = region_container(n);

		p = n;
		if(r->used_blocks < t->used_blocks){
			n = sp_left(n);
			d = LEFT;
		}

		else if(r->used_blocks > t->used_blocks){
			n = sp_right(n);
			d = RIGHT;
		}

		else if(r < t) {
			n = sp_left(n);
			d = LEFT;
		}

		else {
			n = sp_right(n);
			d = RIGHT;
		}
	}

	sp_insert(&sp_root(root_region), p, &r->sp_n, d);
}

static void __region_remove(struct block_region *r)
{
	sp_remove(&sp_root(root_region), &r->sp_n);
}

static void __update_regions(struct block_region *r)
{
	__region_remove(r);
	__region_insert(r);
}

struct mem_region *get_mem_node()
{
	struct sp_node *n = sp_root(root_region);

	while(n){
		struct block_region *r = region_container(n);

		if(r->used_blocks != MAX_BLOCKS){
			r->used_blocks++;
			__update_regions(r);

			return __find_free_block(r);
		}

		n = sp_left(n);
	}

	/* we need to allocate a new region */
	struct block_region *r = __create_region();
	r->used_blocks++;
	__region_insert(r);

	return __find_free_block(r);
}

void free_mem_node(struct mem_region *m)
{
	struct block_wrapper *w = block_container(m);
	w->status = FREE;

	struct block_region *r = block_region(w);
	r->used_blocks--;

	__update_regions(r);
}
