/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file pmem.c
 * Physical memory subsystem. Allocates physical memory pages, with support for
 * different ordered pages, depending on the underlying architecture.
 *
 * Effectively, each page order (4096, 2M, 1G...) has a free list of bitmaps for
 * each page size. When an order runs out of free nodes, it just 'allocates' a
 * node from a higher order list, and gives out those maps. This turned out to
 * be around 50% faster than the previous method, with about half the necessary
 * code. Could still probably be cleaned up a little bit, in particular I don't
 * really care for probe_pmap() vs populate_pmap() but I suppose it's fine for
 * now.
 */

#include <kmi/pmem.h>
#include <kmi/dmem.h>
#include <kmi/debug.h>
#include <kmi/initrd.h>
#include <kmi/regions.h>
#include <kmi/string.h> /* memset */
#include <kmi/bits.h> /* is_nset etc */
#include <libfdt.h>

/* \todo add memory page counting?
 * To make sure memory is not overcommited at clone, for example.
 */

/**
 * Loop over orders in reverse, starting with highest, giving the iterator the
 * name \p iter.
 *
 * @param iter Name of iterator.
 */
#define reverse_foreach_order(iter) \
	for (enum mm_order iter = max_order(); iter != MM_MIN; --iter)

/** Page bitmap node */
struct mm_bmap {
	/** How many bits this node has. This is generally the same as \c bits in
	 * mm_bucket, but could be used for trailing nodes with some irregular
	 * number of bits. */
	size_t size;

	/** How many pages are currently used */
	size_t used;

	/** Next node in freelist */
	struct mm_bmap *next;

	/** Previous node in freelist */
	struct mm_bmap *prev;

	/** Actual bitmap */
	uint8_t bits[];
};

/** Bucket of bitmaps for some order of pages */
struct mm_bucket {
	/** How many bits per (regular) bitmap, see \c size in mm_bmap */
	size_t bits;

	/** Size in bytes of a page of this order */
	size_t page_size;

	/** Current head of freelist */
	struct mm_bmap *head;

	/** Bitmaps in contiguous array, to make populating easier. */
	struct mm_bmap bmap[];
};

/** Physical map. */
struct mm_pmap {
	/** Base address of our map. Note that this should be the virtual base
	 * address of the physical ram. */
	pm_t base;
	/** Buckets, one per order up to maximum order. */
	struct mm_bucket *buckets[MM_NUM];
};

/** Static physical map address. \note If I support NUMA, this should probably not be
 * static, rather one physical map per NUMA region. */
static struct mm_pmap *pmap = 0;

/**
 * Zero out memory if \p populate is true.
 * Helper for populate_pmap(), makes it a bit more easy to follow when we're
 * just calculating the size of out physical map versus actually building it.
 *
 * @param populate Whether to write anything.
 * @param cont Where to write.
 * @param size How many bytes to write.
 * @return Address following last written byte.
 */
static pm_t __zero_if(bool populate, pm_t cont, size_t size)
{
	if (populate)
		memset((void *)cont, 0, size);

	return cont + size;
}

/**
 * Get size in bytes of (regular) bitmap node in this bucket.
 *
 * @param bucket Bucket.
 * @return Size in bytes of (regular) bitmap node.
 */
static size_t __get_set_size(struct mm_bucket *bucket)
{
	return align_up(sizeof(bucket->bmap[0])
	                + (bucket->bits + 1) / sizeof(void *),
	                sizeof(void *));
}

/**
 * Get pointer to bitmap node at index \p set.
 *
 * @param bucket Bucket.
 * @param set Index of bitmap node to get.
 * @return Pointer to bitmap node.
 */
static struct mm_bmap *__get_set(struct mm_bucket *bucket, size_t set)
{
	uintptr_t bmap = (uintptr_t)bucket->bmap;
	return (struct mm_bmap *)(bmap + __get_set_size(bucket) * set);
}

/**
 * Get index from pointer to \p bmap within \p bucket.
 *
 * @param bucket Bucket.
 * @param bmap Bitmap node.
 * @return Index of \p bmap within \p bucket.
 */
static size_t __get_set_index(struct mm_bucket *bucket, struct mm_bmap *bmap)
{
	size_t s = (uintptr_t)bmap - (uintptr_t)bucket->bmap;
	return s / __get_set_size(bucket);
}

/**
 * Attach bitmap \p bmap to freelist within \p bucket.
 *
 * @param bucket Bucket.
 * @param bmap Bitmap node.
 */
static void __attach_set(struct mm_bucket *bucket, struct mm_bmap *bmap)
{
	/* already attached */
	if (bmap->next || bucket->head == bmap)
		return;

	bmap->next = bucket->head;
	bucket->head = bmap;
	if (bmap->next)
		bmap->next->prev = bmap;
}

/**
 * Remove bitmap \p bmap from freelist within \p bucket.
 *
 * @param bucket Bucket.
 * @param bmap Bitmap node.
 */
static void __detach_set(struct mm_bucket *bucket, struct mm_bmap *bmap)
{
	if (bucket->head == bmap)
		bucket->head = bmap->next;

	if (bmap->next)
		bmap->next->prev = bmap->prev;

	if (bmap->prev)
		bmap->prev->next = bmap->next;

	bmap->next = NULL;
	bmap->prev = NULL;
}

/**
 * Calculate address of page within bucket.
 *
 * @param bucket Bucket.
 * @param s Index of bitmap node.
 * @param b Bit within bitmap.
 * @return Address of corresponding page.
 */
static pm_t __page_addr(struct mm_bucket *bucket, size_t s, size_t b)
{
	return pmap->base
	       + s * bucket->page_size * bucket->bits
	       + b * bucket->page_size;
}

/**
 * Calculate which bitmap node index and bit within bitmap an address
 * corresponds to.
 *
 * @param bucket Bucket.
 * @param a Address.
 * @param s Corresponding bitmap node index.
 * @param b Bitmap node bit index.
 */
static void __get_bit(struct mm_bucket *bucket, pm_t a, size_t *s, size_t *b)
{
	a -= pmap->base;
	size_t p = a / bucket->page_size;
	*s = p / bucket->bits;
	*b = p % bucket->bits;
}

/** Counter for how many bytes are currently in use. */
static size_t used = 0;

/**
 * Non-usage counting worker for \ref free_page().
 *
 * @param order Order of page to free.
 * @param addr Address of page to free.
 * @return \ref true if usage count should be updated, \ref false otherwise.
 */
static bool __free_page(enum mm_order order, pm_t addr)
{
	struct mm_bucket *bucket = pmap->buckets[order];
	if (!bucket)
		return false;

	size_t set = 0, bit = 0;
	__get_bit(bucket, addr, &set, &bit);

	struct mm_bmap *bmap = __get_set(bucket, set);
	bmap->used--;

	assert(bitmap_is_set(bmap->bits, bit));
	bitmap_clear(bmap->bits, bit);
	__attach_set(bucket, bmap);

	if (bmap->used == 0) {
		__detach_set(bucket, bmap);

		if (bmap->size == order_width(order + 1))
			__free_page(order + 1, __page_addr(bucket, set, bit));
	}

	return true;
}

void free_page(enum mm_order order, pm_t addr)
{
	if (__free_page(order, addr))
		used -= order_size(order);
}

/**
 * Non-usage counting worker for \ref alloc_page().
 *
 * @param order Order of page to allocate.
 * @return Address of allocated page, NULL of no page available.
 */
static pm_t __alloc_page(enum mm_order order)
{
	struct mm_bucket *bucket = pmap->buckets[order];
	if (!bucket)
		return 0;

	struct mm_bmap *bmap = bucket->head;
	if (!bmap) {
		pm_t a = __alloc_page(order + 1);
		if (!a)
			return 0;

		size_t set = 0, bit = 0;
		__get_bit(bucket, a, &set, &bit);

		bmap = __get_set(bucket, set);
		bmap->size = order_width(order + 1);

		bmap->next = NULL;
		bmap->prev = NULL;
		bmap->used = 0;
		bitmap_clear_all(bmap->bits, bmap->size);
		__attach_set(bucket, bmap);
		return __alloc_page(order);
	}

	bmap->used++;

	size_t set = __get_set_index(bucket, bmap);
	size_t bit = bitmap_find_first_unset(bmap->bits, bmap->size);
	bitmap_set(bmap->bits, bit);

	if (bmap->used == bmap->size)
		__detach_set(bucket, bmap);

	return __page_addr(bucket, set, bit);
}

pm_t alloc_page(enum mm_order order)
{
	pm_t page = __alloc_page(order);
	if (page)
		used += order_size(order);

	return page;
}

/**
 * Non-usage counting worker for \ref __mark_used().
 *
 * @param order Order of page to mark used.
 * @param addr Address of page to mark used.
 * @return \ref true if usage count should be updated, \ref false otherwise.
 */
static bool __mark_used(enum mm_order order, pm_t addr)
{
	struct mm_bucket *bucket = pmap->buckets[order];
	if (!bucket)
		return false;

	size_t set = 0, bit = 0;
	__get_bit(bucket, addr, &set, &bit);

	struct mm_bmap *bmap = __get_set(bucket, set);
	if (bmap->used == 0) {
		bitmap_clear_all(bmap->bits, bmap->size);
		__attach_set(bucket, bmap);
		__mark_used(order + 1, addr);
	}

	/* a page already in use can just be left alone. This MIGHT hide some
	 * bugs in case two separate things overlap in memory during
	 * initialization, but that scenario should probably be handled outside
	 * of this function anyway. */
	if (bitmap_is_set(bmap->bits, bit))
		return false;

	bmap->used++;
	bitmap_set(bmap->bits, bit);

	if (bmap->used == bmap->size)
		__detach_set(bucket, bmap);

	return true;
}

void mark_used(enum mm_order order, pm_t addr)
{
	if (__mark_used(order, addr))
		used += order_size(order);
}

size_t query_used()
{
	return used;
}

/**
 * Helper for probing/populating a bucket.
 *
 * @param n How many pages in total to account for.
 * @todo currently may cut off some pages if \p n is larger than but not a multiple of order
 * width.
 * @param cont Where to place bucket.
 * @param order Order of bucket to populate.
 * @param first First bucket being populated. Top bucket, owns all pages to
 * start with.
 * @param populate Whether to actually write bucket to memory.
 * @return Address right after where last byte of bycket would be.
 */
static pm_t __maybe_populate_bucket(size_t n, pm_t cont, enum mm_order order,
                                    bool first, bool populate)
{
	struct mm_bucket *bucket = (struct mm_bucket *)cont;
	/* todo max order? */
	size_t bits = order_width(order + 1);
	if (bits == 0)
		bits = n;

	if (populate) {
		bucket->bits = bits;
		bucket->page_size = order_size(order);
		bucket->head = NULL;
	}

	size_t set_size = align_up(sizeof(struct mm_bmap)
	                           + (bits + 1) / sizeof(void *),
	                           sizeof(void *));

	cont += sizeof(struct mm_bucket);

	size_t sets = n / bits;
	for (size_t i = 0; i < sets; ++i) {
		struct mm_bmap *bmap = (struct mm_bmap *)cont;
		if (populate) {
			memset(bmap, 0, set_size);
			bmap->size = bits;
		}

		if (first && populate)
			__attach_set(bucket, bmap);

		n -= bits;
		cont += set_size;
	}

	if (n) {
		struct mm_bmap *bmap = (struct mm_bmap *)cont;
		if (populate) {
			memset(bmap, 0, set_size);
			bmap->size = n;
		}

		if (first && populate)
			__attach_set(bucket, bmap);

		cont += set_size;
	}

	return cont;
}

/**
 * Probe how many bytes the physical map would take up, optionally populate
 * empty physical map if \p populate is given.
 *
 * I realize it sounds like two
 * different functions, and that it might be a good idea to split in twine,
 * but my thinking was that using the same algorithm with a
 * flag to enable writing to memory would decrease chances that I would
 * calculate the size differently from what is actually needed. We need an
 * accurate estimate of the pmap size to know whether we can place it somewhere
 * and not overwrite something else.
 *
 * @param ram_base Address in kernel space where the physical RAM starts.
 * @param ram_size Size of RAM in bytes.
 * @param start Where to start building pmap.
 * @param populate Whether to actually write anything out to memory.
 * @return Size of pmap in bytes.
 */
static pm_t __maybe_populate_pmap(pm_t ram_base, size_t ram_size, pm_t start,
                                  bool populate)
{
	pm_t cont = start;

	pmap = (struct mm_pmap *)start;
	cont = __zero_if(populate, cont, sizeof(*pmap));
	if (populate)
		pmap->base = ram_base;

	bool first = true;
	reverse_foreach_order(iter) {
		size_t num = ram_size / order_size(iter);
		if (num == 0)
			continue;

		if (populate)
			pmap->buckets[iter] = (struct mm_bucket *)cont;

		cont = __maybe_populate_bucket(num, cont, iter, first,
		                               populate);
		first = false;
	}

	return cont - start;
}

pm_t populate_pmap(pm_t ram_base, size_t ram_size, pm_t start)
{
	return __maybe_populate_pmap(ram_base, ram_size, start, true);
}

pm_t probe_pmap(pm_t ram_base, size_t ram_size, pm_t start)
{
	return __maybe_populate_pmap(ram_base, ram_size, start, false);
}

/**
 * Helper function for marking area used.
 *
 * @param base Base address of area.
 * @param top Top address of top.
 */
static void __mark_area_used(pm_t base, pm_t top)
{
	if (top < base) {
		bug("top < base: %lx < %lx\n", top, base);
		return;
	}

	size_t area_left = top - base;
	pm_t runner = base;
	while (area_left >= BASE_PAGE_SIZE) {
		mark_used(BASE_PAGE, runner);
		runner += BASE_PAGE_SIZE;
		area_left -= BASE_PAGE_SIZE;
	}

	if (area_left != 0)
		mark_used(BASE_PAGE, runner);
}

/** Helper for keeping track of which memory regions to avoid placing data into. */
struct avoid_region {
	/** Base address of region. */
	pm_t base;

	/** Size of region. */
	pm_t size;
};

/**
 * @param base1
 * @param size1
 * @param base2
 * @param size2
 * @return \ref true if the memory regions indicated have some overlap,
 * \ref false otherwise.
 */
static bool overlaps(pm_t base1, pm_t size1, pm_t base2, pm_t size2)
{
	bool b = base1 >= base2 && base1 < base2 + size2;
	bool t = base1 + size1 > base2 && base1 + size1 <= base2 + size2;
	return b || t;
}

/**
 * Mark reserved memory region used, to avoid it getting accidentally allocated.
 *
 * @param ram_base RAM base address in kernelspace.
 * @param ram_size RAM size.
 * @param avoid_count How many regions to avoid.
 * @param avoid Regions to avoid. Should all be in kernelspace, everything
 * outside RAM is ignored.
 */
static void __mark_reserved(pm_t ram_base, pm_t ram_size, size_t avoid_count,
                            struct avoid_region avoid[64])
{
	for (size_t i = 0; i < avoid_count; ++i) {
		pm_t base = avoid[i].base;
		pm_t size = avoid[i].size;

		if (!overlaps(base, size, ram_base, ram_size))
			continue;

		pm_t top = base + size;
		__mark_area_used(base, top);
		info("marked [%lx - %lx] reserved\n", base, top);
	}
}

/**
 * Figure out what if any regions of RAM are reserved and append them to \p
 * avoid.
 *
 * @param exists How many elements are currently in \p avoid.
 * @param avoid Array of regions to avoid, must not be sparse.
 * @param fdt Flattened device tree.
 * @return Total number of regions avoid.
 */
static size_t build_reserved_map(size_t exists, struct avoid_region avoid[64],
                                 void *fdt)
{
	int rmem_offset = fdt_path_offset(fdt, "/reserved-memory");
	struct cell_info ci = get_cellinfo(fdt, rmem_offset);

	int node = 0;
	fdt_for_each_subnode(node, fdt, rmem_offset) {
		uint8_t *rmem_reg = (uint8_t *)fdt_getprop(fdt, node, "reg",
		                                           NULL);

		pm_t base = (pm_t)fdt_load_reg_addr(ci, rmem_reg, 0);
		pm_t size = (pm_t)fdt_load_reg_size(ci, rmem_reg, 0);

		avoid[exists++] = (struct avoid_region){(pm_t)__va(base), size};
		assert(exists < 64);
	}

	return exists;
}

/**
 * Selects a fitting base address for a block of memory of size \p size, while
 * avoiding overwriting anything in \p avoid.
 * Addresses to be avoided are allowed to be outside of RAM,
 * for example in ROM, in which case they are just ignored.
 *
 * @param ram_base Base address of RAM.
 * @param ram_size Size of RAM.
 * @param size Size of block to find.
 * @param avoid_count How many elements in the \p avoid array.
 * @param avoid Which memory regions to avoid.
 * @return A suitable base address, aligned on a word boundary.
 */
static pm_t select_base(pm_t ram_base, pm_t ram_size, pm_t size,
                        pm_t avoid_count,
                        struct avoid_region avoid[64])
{
	for (size_t i = 0; i < avoid_count; ++i) {
		/* try placing things just after each avoidance region */
		pm_t base = avoid[i].base + avoid[i].size;

		/* any better alignment than this has to be manually handled
		 * outside of this function */
		base = align_up(base, sizeof(long));

		if (!overlaps(base, size, ram_base, ram_size))
			continue;

		for (size_t a = 0; a < avoid_count; ++a) {
			if (overlaps(base, size, avoid[a].base, avoid[a].size))
				goto retry;

		}

		return base;
retry:
		continue;
	}

	return 0;
}

/** Actual kernel size in bytes. Populated by `_start`, as I don't think we
 * have enough control from C to get both LLVM and GCC to output correct code
 * if this was just a virtual symbol defined in a linker file. */
size_t kernel_size = 0;

void init_pmem(void *fdt, uintptr_t load_addr)
{
	/** @todo should I keep the info outputs? I suppose it's nice to see
	 * if any assumption is being broken in the serial log, but in that case
	 * I should really try adding more of them to other parts of the
	 * codebase as well, the pmem subsystem isn't really especially complex.
	 */
	info("initializing pmem\n");

	/* here it's a bit easier to work with virtual RAM addresses, but
	 * physical ones could work just as well. */
	pm_t ram_base = (pm_t)__va(get_ram_base());
	pm_t ram_size = get_ram_size();

	info("using ram range [%lx - %lx]\n",
	     ram_base, ram_base + ram_size);

	/** @todo could probably improve error messages on failing to get fdt
	 * values */
	pm_t initrd_base = get_initrdbase(fdt);
	pm_t initrd_size = get_initrdsize(fdt);
	info("found initrd at [%lx - %lx]\n", initrd_base,
	     initrd_base + initrd_size);

	assert(is_aligned(initrd_base, BASE_PAGE_SIZE));

	pm_t fdt_base = (pm_t)fdt;
	pm_t fdt_size = fdt_totalsize(fdt);
	info("found fdt at [%lx - %lx]\n", fdt_base, fdt_base + fdt_size);

	assert(is_aligned(fdt_base, BASE_PAGE_SIZE));

	/* find probably most suitable contiguous region of ram for our physical
	 * ram map  */

	size_t probe_size = probe_pmap(0, ram_size, 0);
	info("pmem map probe size returned %lu\n", probe_size);

	/* avoidance regions, note that stack and so on is included in the
	 * kernel. Addresses can be outside RAM, in which case they are just
	 * ignored. */
	struct avoid_region avoid[64] = {
		{(pm_t)__va(load_addr), kernel_size},
		{(pm_t)__va(initrd_base), initrd_size},
		{(pm_t)__va(fdt_base), fdt_size}
	};

	size_t avoid_count = build_reserved_map(4, avoid, fdt);
	pm_t pmap_base = select_base(ram_base, ram_size,
	                             probe_size, avoid_count, avoid);

	assert(pmap_base);
	info("choosing to place pmem map at %lx\n", pmap_base);

	size_t actual_size = populate_pmap(ram_base, ram_size, pmap_base);
	info("pmem map actual size %lu\n", actual_size);

	if (probe_size != actual_size) {
		bug("probe_size (%#lx) != actual_size (%#lx)\n", probe_size,
		    actual_size);
	}

	avoid[avoid_count++] = (struct avoid_region){pmap_base, actual_size};

	/* mark reserved mem */
	__mark_reserved(ram_base, ram_size, avoid_count, avoid);

	init_mem_nodes();

	init_devmem((pm_t)__pa(ram_base), (pm_t)__pa(ram_base + ram_size));
}
