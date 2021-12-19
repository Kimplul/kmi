#include <apos/vmem.h>
#include <apos/mem_nodes.h>

#define mark_region_used(r) ((r) = 1)
#define mark_region_unused(r) ((r) = 0)
#define is_region_used(r) (r)

static struct sp_root free_regions = (struct sp_root){0};
static struct sp_root used_regions = (struct sp_root){0};

/* pretty major slowdown when we get to some really massive numbers, not
 * entirely sure why. Will need to check up on this at some point, have I
 * somehow managed to come up with a _very_ bad situation for my sp_trees?
 *
 * EDIT: apparently, yeah. Max depth of 106 with a million entries, interesting.
 * I guess since in this scenario all sizes are 1, and I just shove everything
 * to the right? Maybe?
 *
 * EDIT upon EDIT: yeah, when taking the start position of the region into
 * account we get a much more sensible max depth of 39 for 5 million entries.
 * Seems I have found a weakness in sp_trees :D
 *
 * Duplicate entries don't work well with any trees, I think. Good to know,
 * maybe not even anything with sp_trees but more a weakness of binary trees in
 * general?
 */
static struct sp_mem *sp_free_insert_region(struct sp_mem *m)
{
	struct sp_node *n = sp_root(free_regions), *p = NULL;
	size_t start = m->start;
	size_t size = m->end - m->start;
	enum sp_dir d = LEFT;

	m->sp_n = (struct sp_node){0};

	while(n){
		struct sp_mem *t = mem_container(n);
		size_t nsize = t->end - t->start;
		p = n;

		if(size < nsize){
			n = sp_left(n);
			d = LEFT;
		}

		else if(size > nsize) {
			n = sp_right(n);
			d = RIGHT;
		}

		else if (start < t->start){
			n = sp_left(n);
			d = LEFT;
		}

		else {
			n = sp_right(n);
			d = RIGHT;
		}
	}

	if(sp_root(free_regions))
		sp_insert(&sp_root(free_regions), p, &m->sp_n, d);
	else
		sp_root(free_regions) = &m->sp_n;

	return m;
}

static struct sp_mem *sp_used_insert_region(struct sp_mem *m)
{
	struct sp_node *n = sp_root(used_regions), *p = NULL;
	vm_t start = m->start;
	enum sp_dir d = LEFT;

	m->sp_n = (struct sp_node){0};

	while(n){
		struct sp_mem *t = mem_container(n);

		p = n;

		if(start < t->start){
			n = sp_left(n);
			d = LEFT;
		}

		else {
			/* we should never encounter a situation where start =
			 * t->start */
			n = sp_right(n);
			d = RIGHT;
		}
	}

	if(sp_root(used_regions))
		sp_insert(&sp_root(used_regions), p, &m->sp_n, d);
	else
		sp_root(used_regions) = &m->sp_n;

	return m;
}

int sp_mem_init(size_t arena_size)
{
	struct sp_mem *m = get_mem_node();
	m->end = arena_size;
	sp_free_insert_region(m);

	return 0;
}

static void __sp_mem_destroy(struct sp_node *n)
{
	if(!n)
		return;

	__sp_mem_destroy(sp_left(n));
	__sp_mem_destroy(sp_right(n));

	struct sp_mem *m = mem_container(n);
	free_mem_node(m);
}

void sp_mem_destroy()
{
	__sp_mem_destroy(sp_root(free_regions));
	__sp_mem_destroy(sp_root(used_regions));
}

/* interestingly this is now the main bottleneck :D
 *
 * eh, it's not a massive thing I guess, maybe the code could be a bit quicker
 * but I mean 10 000 000 memory allocations in 20 s is good enough for now
 * */
static struct sp_mem *sp_used_find(vm_t start)
{
	struct sp_node *n = sp_root(used_regions);
	while(n){
		struct sp_mem *t = mem_container(n);
		if(start == t->start)
			return t;

		if(start < t->start)
			n = sp_left(n);
		else
			n = sp_right(n);
	}

	return 0;
}

static struct sp_mem *sp_mem_create_region(vm_t start, vm_t end,
		struct sp_mem *prev, struct sp_mem *next)
{
	struct sp_mem *m = get_mem_node();
	m->start = start;
	m->end = end;
	m->prev = prev;
	m->next = next;
	return m;
}

static struct sp_mem *sp_free_find_first(size_t size, size_t alignment)
{
	struct sp_node *n = sp_root(free_regions);
	while(n){
		struct sp_mem *t = mem_container(n);
		size_t nsize = t->end - align_up(t->start, alignment);

		if(size <= nsize)
			return t;

		n = sp_right(n);
	}

	return 0;
}

/* apparently Linux doesn't necessarily give a shit about mmap hints, so I'll
 * just ignore them for now. Note that alloc_region should only be used when
 * mmap is called with MAP_ANON, all other situations should be handled in some
 * fs server */
vm_t alloc_region(size_t size, size_t alignment)
{
	struct sp_mem *m = sp_free_find_first(size, alignment);
	if(!m)
		return 0;

	sp_remove(&sp_root(free_regions), &m->sp_n);

	vm_t aligned_start = align_up(m->start, alignment);

	vm_t pre_start = m->start;
	vm_t pre_end = aligned_start;

	vm_t start = pre_end;
	vm_t end = aligned_start + size;

	vm_t post_start = end;
	vm_t post_end = m->end;

	if(pre_start != pre_end){
		struct sp_mem *n = sp_mem_create_region(pre_start, pre_end, m->prev, m);
		m->prev = n;
		if(n->prev)
			n->prev->next = n;

		sp_free_insert_region(n);
	}

	if(post_start != post_end){
		struct sp_mem *n = sp_mem_create_region(post_start, post_end, m, m->next);
		m->next = n;
		if(n->next)
			n->next->prev = n;

		sp_free_insert_region(n);
	}

	m->end = end;
	m->start = start;
	mark_region_used(m->flags);
	sp_used_insert_region(m);
	return start;
}

static void __sp_try_coalesce_prev(struct sp_mem *m)
{
	while(m){
		if(!m || is_region_used(m->flags))
			return;

		struct sp_mem *p = m->prev;
		if(!p || is_region_used(p->flags))
			return;

		m->start = p->start;
		m->prev = p->prev;

		if(m->prev)
			m->prev->next = m;

		sp_remove(&sp_root(free_regions), &p->sp_n);
		free_mem_node(p);

		m = m->prev;
	}
}

static void __sp_try_coalesce_next(struct sp_mem *m)
{
	while(m){
		if(!m || is_region_used(m->flags))
			return;

		struct sp_mem *n = m->next;
		if(!n || is_region_used(n->flags))
			return;

		m->end = n->end;
		m->next = n->next;

		if(m->next)
			m->next->prev = m;

		sp_remove(&sp_root(free_regions), &n->sp_n);
		free_mem_node(n);

		m = m->next;
	}
}

static void sp_mem_try_coalesce(struct sp_mem *m)
{
	__sp_try_coalesce_prev(m);
	__sp_try_coalesce_next(m);
}

void free_region(vm_t start)
{
	struct sp_mem *m = sp_used_find(start);
	if(!m)
		return;

	sp_remove(&sp_root(used_regions), &m->sp_n);
	mark_region_unused(m->flags);

	sp_mem_try_coalesce(m);
	sp_free_insert_region(m);
}
