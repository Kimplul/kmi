#include <apos/vmem.h>
#include <apos/pmem.h>
#include <apos/mem.h>
#include <apos/string.h>

/* new idea, not implemented:
 * region(free) -> region(not free) -> region(free) -> region(free) ...
 *
 * each region comes right after the next, would at least be pretty quick to 
 * merge free blocks?
 */

enum mm_block_status_t {
	FREE, USED
};

struct mm_block_region_t;

/* linked list for now because I'm shit at coding */
struct mm_block_t {
	enum mm_block_status_t status;
	vm_t start;
	vm_t end;
	struct mm_block_t *next;
	struct mm_block_t *prev;
};

struct mm_block_region_t {
	vm_t vaddr;
	size_t blocks;
	size_t max_blocks;
	struct mm_block_region_t *next;
	struct mm_block_t *first;
};

static struct mm_block_region_t *root_region = (struct mm_block_region_t *)ROOT_REGION;
static struct mm_block_t *root_block = 0;

#define NODE_REGION(x) ((struct mm_block_region_t *)(((vm_t)x) & (__mm_page_shift - 1)))
#define NEXT_BLOCK() ((struct mm_block_region_t *)\
((region_counter * __o_size(MM_O0)) + ROOT_PTE))

/* TODO: mark all used regions, also blocks */
static struct mm_block_t *get_free_block(struct vm_branch_t *branch)
{
	struct mm_block_region_t *region = root_region;
	size_t region_counter = 1;
	for(; region; region = region->next){
		if(region->next == 0){
			pm_t next_pa = alloc_page(MM_O0, 0);
			struct mm_block_region_t *next_va = NEXT_BLOCK();

			map_vmem(branch, next_pa, (vm_t)next_va,
					VM_W | VM_R | VM_V, MM_O0);

			memset(next_va, 0, __o_size(MM_O0));
			next_va->max_blocks =
				(__o_size(MM_O0) - sizeof(struct mm_block_region_t))
				/ sizeof(struct mm_block_t);

			region->next = next_va;
		}

		if(region->blocks == region->max_blocks)
			continue;

		struct mm_block_t *block = region->first;
		for(size_t i = 0; i < region->max_blocks; ++i){
			if(block[i].start == 0 && block[i].end == 0){
				NODE_REGION(&block[i])->blocks++;
				return &block[i];
			}
		}

		region_counter++;
	}

	return 0;
}

void init_vmem(struct vm_branch_t *branch, vm_t tmp_pte)
{
#if defined(KERNEL)
	arch_init_vmem(branch, tmp_pte);
#else
	(void)tmp_pte;
#endif

	pm_t first_block = alloc_page(MM_O0, 0);
	map_vmem(branch, first_block, (vm_t)root_region, VM_W | VM_R | VM_V, MM_O0);
	memset(root_region, 0, __o_size(MM_O0));

	root_region->max_blocks = (__o_size(MM_O0) - sizeof(struct mm_block_region_t))
		/ sizeof(struct mm_block_t);
	root_region->first = (struct mm_block_t *)sizeof(struct mm_block_t);
	root_block = root_region->first;

	root_block->start = __o_size(MM_O0);
	/* TODO: add in UMEM_TOP or something */
	root_block->end = -1;
	root_block->status = USED;
}


/* ... new_node -> node ... */
static void insert_before(struct vm_branch_t *branch, struct mm_block_t *node, vm_t split)
{
	struct mm_block_t *new_node = get_free_block(branch);

	new_node->start = node->start;
	new_node->end = split;

	node->start = split;

	struct mm_block_t *prev = node->prev;
	new_node->next = node;
	new_node->prev = prev;
	prev->next = new_node;
	node->prev = new_node;
}

/* ... node -> new_node ... */
static void insert_after(struct vm_branch_t *branch, struct mm_block_t *node, vm_t split)
{
	struct mm_block_t *new_node = get_free_block(branch);

	new_node->start = split;
	new_node->end = node->end;

	node->end = split;

	struct mm_block_t *next = node->next;
	new_node->next = next;
	new_node->prev = node;
	next->prev = new_node;
	node->next = new_node;
}

static void gobble_block(struct vm_branch_t *branch, struct mm_block_t *node,
		vm_t start, vm_t end)
{
	node->status = USED;

	/* ... node ... */
	if(node->start == start && node->end == end)
		return;

	/* ... node -> new_node ... */
	if(node->start == start && node->end >= end){
		insert_after(branch, node, end);
		struct mm_block_t *new = node->next;

		new->status = FREE;
		return;
	}

	/* ... new_node -> node ...*/
	if(node->start < start && node->end == end){
		insert_before(branch, node, start);
		struct mm_block_t *new = node->prev;

		new->status = FREE;
		return;
	}

	/* ... new_node1 -> node -> new_node2 .. */
	insert_before(branch, node, start);
	struct mm_block_t *prev = node->prev;

	insert_after(branch, node, end);
	struct mm_block_t *next = node->next;

	prev->status = FREE;
	next->status = FREE;
}

/* only map with 4K blocks to keep it simple for now */
vm_t map_vregion(struct vm_branch_t *branch, pm_t base, vm_t start, size_t size,
		uint8_t flags)
{
	struct mm_block_t *node = root_block;
	for(; node; node = node->next){
		if(node->start > start)
			return 0;

		if(node->status == FREE && node->end >= start + size){
			gobble_block(branch, node, start, start + size);
			break;
		}
	}

	for(; size >= __o_size(MM_O0); size -= __o_size(MM_O0)){
		map_vmem(branch, start, base, flags, MM_O0);
		start += __o_size(MM_O0);
		base += __o_size(MM_O0);
	}

	return start;
}

static void free_block(struct mm_block_t *node)
{
	struct mm_block_t *prev = node->prev;
	struct mm_block_t *next = node->next;

	if(prev->status == FREE && next->status == FREE){
		/* merge all three blocks */
		prev->end = next->end;
		prev->next = next->next;
		next->next->prev = prev;

		node->start = 0;
		node->end = 0;

		next->start = 0;
		next->end = 0;

		NODE_REGION(node)->blocks--;
		NODE_REGION(next)->blocks--;
		return;
	}

	if(prev->status == FREE){
		prev->end = node->end;
		prev->next = next;
		next->prev = prev;

		node->start = 0;
		node->end = 0;

		NODE_REGION(node)->blocks--;
		return;
	}

	if(next->status == FREE){
		next->start = node->start;
		next->prev = prev;
		prev->next = next;

		node->start = 0;
		node->end = 0;

		NODE_REGION(node)->blocks--;
		return;
	}

	node->status = FREE;
}

void unmap_vregion(struct vm_branch_t *branch, vm_t start)
{
	size_t size = 0;
	struct mm_block_t *node = root_block;
	for(; node; node = node->next){
		/* if node->start == start status should be USED in all cases,
		 * but let's just go with this
		 */
		if(node->status == USED && node->start == start){
			size = node->end - node->start;
			free_block(node);
		}
	}

	for(; size >= __o_size(MM_O0); size -= __o_size(MM_O0)){
		unmap_vmem(branch, start, MM_O0);
		start += __o_size(MM_O0);
	}
}
