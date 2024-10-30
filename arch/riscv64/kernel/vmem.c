/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file vmem.c
 * riscv64 implementation of arch-specific virtual memory handling.
 */

#include <kmi/assert.h>
#include <kmi/string.h>
#include <kmi/pmem.h>
#include <kmi/vmem.h>
#include <kmi/mem.h>
#include <kmi/debug.h>
#include <arch/cpu.h>
#include "pages.h"
#include "arch.h"
#include "csr.h"

/**
 * Get page table entry physical page number.
 *
 * @param pte Page table entry.
 * @return Corresponding physical page number.
 */
#define pte_ppn(pte) (((pm_t)(pte)) >> 10)

/**
 * Get page table entry flags.
 *
 * @param pte Page table entry.
 * @return Corresponding flags.
 */
#define pte_flags(pte) (((pm_t)(pte)) & 0xff)

/**
 * Convert physical memory address to page table entry.
 *
 * @param p Physical memory address.
 * @param f Flags to use.
 * @return Corresponding page table entry.
 */
#define to_pte(p, f) ((((p) >> page_shift()) << 10) | (f))

/**
 * Get physical address in page table entry.
 *
 * @param pte Page table entry.
 * @return Corresponding physical address.
 */
#define pte_paddr(pte) (pte_ppn(pte) << page_shift())

/**
 * Get virtual address in page table entry.
 *
 * @param pte Page table entry.
 * @return Corresponding virtual address.
 */
#define pte_addr(pte) __va(pte_paddr(pte))

/**
 * Virtual memory address to page order index.
 *
 * @param a Virtual address.
 * @param o Order of page.
 * @return Corresponding page index.
 */
#define vm_to_index(a, o) (pm_to_index(a, o))

/**
 * Check if page table entry is active.
 *
 * @param pte Page table entry.
 * @return \c 0 if entry is not active, non-zero otherwise.
 */
#define is_active(pte) (pte_flags(pte) &VM_V)

/**
 * Check if page table entry is a leaf.
 *
 * @param pte Page table entry.
 * @return \c 0 if entry is not leaf, non-zero otherwise.
 */
#define is_leaf(pte) (is_active(pte) && (pte_flags(pte) & ~VM_V))

/**
 * Check if page table entry is a branch.
 *
 * @param pte Page table entry.
 * @return \c 0 if entry is not branch, non-zero otherwise.
 */
#define is_branch(pte) (is_active(pte) && !(pte_flags(pte) & ~VM_V))

/**
 * Gravestone marker.
 *
 * Riscv allows us to have arbitrary data in page entries, as long as they're
 * not marked active (VM_V) the content is ignored. Here we use this to our
 * advantage by differentiating between empty entries (NULL) and filler entries
 * (GRAVESTONE).
 *
 * A gravestone tells us that somewhere above it (= higher index) there is an
 * active entry. This is useful mainly in \ref clone_uvmem(), where we can stop
 * copying data as soon as we hit an empty entry. I expect typical programs to
 * generally have most active entries in relatively low addresses, and allowing
 * us to skip copying 'obvious' entries is way quicker than copying the whole
 * 2048 byte user virtual memory.
 *
 * Current optimisations also include setting the uvmem to stop on an 8-page
 * boundary, allowing \ref clone_uvmem() to work in eight page increments for a
 * bit of extra speed. Gravestones are only applied to userspace virtual memory,
 * that is kernel and rpc memory regions are ignored.
 *
 * Example of how stuff should look like:
 *
 * Startin with page entries:
 * 1 2 3 4 0 0 0 ...
 *
 * Mapping a page:
 * 1 2 3 4 0 5 0 ...
 *
 * Adding gravestones:
 * 1 2 3 4 G 5 0 ...
 *
 * More testing is probably necessary, as the init tests program doesn't really
 * excercise the mapping utilities.
 */
#define GRAVESTONE VM_G

/**
 * Check if pte is unused, i.e. either a gravestone or empty.
 *
 * @param b pte to check.
 * @return \ref true if \p b is unused.
 */
static bool __unused(pm_t b)
{
	return b == GRAVESTONE || b == NULL;
}

/**
 * Find page table entry corresponding to virtual address.
 *
 * @param b Virtual memory to work in.
 * @param v Virtual address to look for.
 * @param o Address where to return page order to.
 * @return Physical address of page.
 */
static pm_t *__find_vmem(struct vmem *b, vm_t v, enum mm_order *o)
{
	enum mm_order top = max_order();
	if (o)
		*o = MM_O0;
	do {
		size_t idx = vm_to_index(v, top);
		pm_t pte = (pm_t)b->leaf[idx];

		if (__unused(pte))
			return 0;

		if (is_leaf(pte)) {
			if (o)
				*o = top;

			return (pm_t *)&b->leaf[idx];
		}

		b = (struct vmem *)pte_addr(pte);
	} while (top--);

	return 0;
}

stat_t set_vpage_flags(struct vmem *branch, vm_t vaddr, vmflags_t flags)
{
	enum mm_order order;
	pm_t *pte = __find_vmem(branch, vaddr, &order);
	if (pte) {
		set_bits(*pte, vp_flags(flags));
		return OK;
	}

	return ERR_NF;
}

stat_t clear_vpage_flags(struct vmem *branch, vm_t vaddr, vmflags_t flags)
{
	enum mm_order order;
	pm_t *pte = __find_vmem(branch, vaddr, &order);
	if (pte) {
		clear_bits(*pte, vp_flags(flags));
		return OK;
	}

	return ERR_NF;
}

stat_t mod_vpage(struct vmem *branch, vm_t vaddr, pm_t paddr, vmflags_t flags)
{
	flags |= VM_A | VM_D;

	enum mm_order order;
	pm_t *pte = __find_vmem(branch, vaddr, &order);
	if (pte) {
		*pte = to_pte((pm_t)__pa(paddr), vp_flags(flags));
		return OK;
	}

	return ERR_NF;
}

/* huh, should probably add status flags etc. to all my API functions. Damn, I'm
 * lazy. */
stat_t stat_vpage(struct vmem *branch, vm_t vaddr, pm_t *paddr,
                  enum mm_order *order, vmflags_t *flags)
{
	pm_t *pte = __find_vmem(branch, vaddr, order);
	if (pte) {
		if (paddr)
			*paddr = (pm_t)pte_addr(*pte);

		if (flags)
			*flags = pte_flags(*pte);

		return OK;
	}

	return ERR_NF;
}

/**
 * Create virtual memory leaf page table.
 *
 * @return New virtual memory leaf page table.
 */
static struct vmem *__create_leaf()
{
	pm_t new_leaf = alloc_page(MM_KPAGE);
	if (!new_leaf)
		return NULL;

	memset((void *)new_leaf, 0, sizeof(struct vmem));
	return (struct vmem *)to_pte((pm_t)__pa(new_leaf), VM_V);
}

/**
 * Destroy virtual memory page table branch.
 *
 * @param b Virtual memory to work in.
 */
static void __destroy_branch(struct vmem *b)
{
	if (!b)
		return;

	for (size_t i = 0; i < RISCV_NUM_LEAVES; ++i) {
		if (is_branch(b->leaf[i]))
			__destroy_branch((struct vmem *)pte_addr(b->leaf[i]));
	}

	free_page(MM_KPAGE, (pm_t)b);
}

/**
 * Add graves if necessary.
 *
 * Checks that the index is within user virtual memory. If it is, change all
 * NULL-entries to gravestones at lower addresses than \p idx.
 *
 * @param branch Top level branch to add graves to.
 * @param idx Index of new entry just added.
 */
static void __add_graves(struct vmem *branch, size_t idx)
{
	if (idx >= CSTACK_PAGE)
		return;

	for (ssize_t i = idx - 1; i >= 0; --i) {
		if (!__unused((pm_t)branch->leaf[i]))
			return;

		branch->leaf[i] = (struct vmem *)GRAVESTONE;
	}
}

stat_t map_vpage(struct vmem *branch, pm_t paddr, vm_t vaddr, vmflags_t flags,
                 enum mm_order order)
{
	struct vmem *root = branch;
	enum mm_order top = max_order();

	/* eventually we may want to keep track of page accesses,
	 * but for now they're mainly a nuisance. */
	flags |= VM_A | VM_D;

	while (top != order) {
		size_t idx = vm_to_index(vaddr, top);

		if (__unused((pm_t)branch->leaf[idx])) {
			struct vmem *leaf = __create_leaf();
			if (!leaf)
				return ERR_OOMEM;

			branch->leaf[idx] = leaf;
		}

		branch = (struct vmem *)pte_addr(branch->leaf[idx]);
		top--;
	}

	size_t idx = vm_to_index(vaddr, top);
	assert(!is_branch(branch->leaf[idx]));

	branch->leaf[idx] =
		(struct vmem *)to_pte((pm_t)__pa(paddr), vp_flags(flags));

	__add_graves(root, vm_to_index(vaddr, max_order()));
	return OK;
}

/**
 * Remove graves if possible.
 *
 * Checks if \p idx is in user virtual memory. If it is, check if the entry at
 * \p idx was the top page and was turned into a gravestone. If it was, start
 * removing gravestoned until we hit the next top.
 *
 * @param branch Top level branch to remove gravestones in.
 * @param idx Index of just unmapped page at the top level.
 */
static void __remove_graves(struct vmem *branch, size_t idx)
{
	if (idx >= CSTACK_PAGE)
		return;

	if ((pm_t)branch->leaf[idx + 1] != NULL)
		return;

	for (ssize_t i = idx; i >= 0; --i) {
		if ((pm_t)branch->leaf[i] != GRAVESTONE)
			return;

		branch->leaf[i] = NULL;
	}
}

stat_t unmap_vpage(struct vmem *branch, vm_t vaddr)
{
	pm_t *pte = __find_vmem(branch, vaddr, 0);
	if (pte) {
		*pte = GRAVESTONE;
		__remove_graves(branch, vm_to_index(vaddr, max_order()));
		return OK;
	}

	return ERR_NF;
}

void flush_tlb(uintptr_t addr)
{
	__asm__ volatile ("sfence.vma %0, x0\n" : : "r" (addr) : "memory");
}

void flush_tlb_full()
{
	__asm__ volatile ("sfence.vma %0\n" : : "r" (0) : "memory");
}

void flush_tlb_all()
{
	/** @todo this only works on a single core atm. needs to do an IPI */
	__asm__ volatile ("sfence.vma\n" ::: "memory");
}

/**
 * Jump into virtual memory.
 *
 * @param branch Virtual memory address space to jump into.
 * @param m Riscv memory mode to use.
 */
static void __use_vmem(struct vmem *branch, enum mm_mode m)
{
	pm_t satp = branch_to_satp(branch, m);
	csr_write(CSR_SATP, satp);
	flush_tlb_full();
	/** @todo ASID table for maybe faster context switches? */
}

/**
 * Populate \p branch with direct mapping.
 * Mainly intended for bringup and init stuff.
 * Arguably works only for riscv64 and is sort of shared with init.c, so could
 * still be improved.
 *
 * @param branch Branch to populate.
 */
static void __populate_dmap(struct vmem *branch)
{
	size_t flags = VM_V | VM_R | VM_W | VM_X | VM_G | VM_D | VM_A;
	for (size_t i = 0; i < CSTACK_PAGE; ++i)
		branch->leaf[i] =
			(struct vmem *)to_pte(TOP_PAGE_SIZE * i, flags);
}

/** How many base pages we use for the rpc stack. Used fairly often so calculate
 * it at the start and then reference it. */
size_t rpc_pages;

/** Initial stack used when booting the kernel. Probably way overkill, but 4K is
 * such a nice number. */
long riscv_init_stack[4096 / sizeof(long)];

/** Initial page table used when booting the kernel. Has to be aligned properly,
 * and so the kernel itself has to be on a 4K boundary. */
__aligned(4096) struct vmem bootvmem;

/** Page entry for mapping kernel on a O1 page level, similar to how Linux does
 * it. */
__aligned(4096) struct vmem kvmem;

/* adding a third page entry would let us map the kernel at any 4KiB boundary
 * but eh, Linux seems fine with 2MiB so I guess I shall be as well. */

struct vmem *init_mapping(uintptr_t load_addr)
{
	rpc_pages = order_size(MM_O1) / BASE_PAGE_SIZE;
	kvmem.leaf[0] = (struct vmem *)to_pte(load_addr,
	                                      VM_A | VM_G | VM_D | VM_R | VM_W |
	                                      VM_X | VM_V);

	__populate_dmap(&bootvmem);
	populate_kvmem(&bootvmem);
	__use_vmem(&bootvmem, DEFAULT_Sv_MODE);
	return &bootvmem;
}


struct vmem *init_vmem(void *fdt)
{
	UNUSED(fdt);

	struct vmem *b = create_vmem();
	if (!b)
		return NULL;

	__populate_dmap(b);
	/* update which memory branch to use */
	use_vmem(b);
	return b;
}

struct vmem *create_vmem()
{
	struct vmem *b = (struct vmem *)alloc_page(MM_KPAGE);
	if (!b)
		return NULL;

	memset(b, 0, MM_KPAGE_SIZE);
	populate_kvmem(b);
	return b;
}

void use_vmem(struct vmem *b)
{
	__use_vmem(__pa(b), DEFAULT_Sv_MODE);
}

void destroy_vmem(struct vmem *b)
{
	if (!b)
		return;

	/* don't free kernel mapping as that one is guaranteed to be statically
	 * allocated */
	for (size_t i = 0; i < KERNEL_PAGE; ++i) {
		if (is_branch(b->leaf[i]))
			__destroy_branch((struct vmem *)pte_addr(b->leaf[i]));
	}

	free_page(MM_KPAGE, (pm_t)b);
}

void destroy_rpcmem(struct vmem *b)
{
	if (!b)
		return;

	/* only destroy branches that the rpc vmem is sure to own */
	for (size_t i = CSTACK_PAGE; i < KERNEL_PAGE; ++i) {
		if (is_branch(b->leaf[i]))
			__destroy_branch((struct vmem *)pte_addr(b->leaf[i]));
	}

	free_page(MM_KPAGE, (pm_t)b);
}

/**
 * Helper for mapping in the kernel virtual page.
 * Remember that the kernel lives on its own in a 2MiB (riscv64) region at
 * VM_KERNEL, and \ref __va() and \ref __pa() won't directly work on it.
 *
 * @param b Branch to map kernel into.
 */
static void map_kernel(struct vmem *b)
{
	/* slightly worried about this not being guaranteed to be pc relative,
	 * but seems to work at the moment at least. */
	intptr_t addr = (pm_t)&kvmem;

	/* virtual memory is negative, unsure if this is really portable but I
	 * guess it's good enough for us for now */
	if (addr < 0)
		addr = addr - VM_KERNEL + get_load_addr();

	/* branch */
	b->leaf[KERNEL_PAGE] = (struct vmem *)to_pte((pm_t)addr, VM_V);
}

stat_t populate_kvmem(struct vmem *b)
{
	size_t flags = VM_V | VM_R | VM_W | VM_X | VM_G | VM_D | VM_A;
	for (size_t i = KSTART_PAGE; i < KERNEL_PAGE; ++i)
		b->leaf[i] = (struct vmem *)to_pte(
			TOP_PAGE_SIZE * (i - KSTART_PAGE),
			flags);

	/* map in IO region if debugging is specified */
	map_io_dbg(b);

	/* map actual kernel */
	map_kernel(b);
	return OK;
}

#if defined(DEBUG)
vm_t setup_kernel_io(struct vmem *b, vm_t paddr)
{
	pm_t top_page = paddr / TOP_PAGE_SIZE;
	pm_t addr = top_page * TOP_PAGE_SIZE;
	b->leaf[IO_PAGE] = (struct vmem *)to_pte(addr,
	                                         VM_V | VM_R | VM_W | VM_A |
	                                         VM_D);
	/* flush might be necessary when we're in the actual vmem we're
	 * modifying, or during the startup stage where we don't have a tcb yet.
	 * I don't think checking the rpc context is necessary? */
	if (!cur_tcb() || cur_tcb()->proc.vmem == b)
		flush_tlb_full();

	return -TOP_PAGE_SIZE + paddr - addr;
}
#endif

void clone_uvmem(struct vmem *r, struct vmem *b)
{
	size_t i = 0;
	for (; i < CSTACK_PAGE; ++i) {
		struct vmem *t = r->leaf[i + 0];
		if (t == 0)
			break;

		b->leaf[i] = t;
	}

	for (; i < CSTACK_PAGE; ++i) {
		struct vmem *t = b->leaf[i + 0];
		if (t == 0)
			break;

		b->leaf[i] = 0;
	}
}

size_t max_rpc_size()
{
	return SZ_512K;
}

void copy_rpc_stack(struct tcb *t, struct tcb *c)
{
	/* this is of terrible, copying 2MiB for every fork, not
	 * great. TODO: use the direct pointer to the rpc stack element? */
	for (size_t i = 0; i < rpc_pages; ++i) {
		pm_t p1, p2;
		stat_t ok1 = stat_vpage(t->rpc.vmem, RPC_STACK_BASE + BASE_PAGE_SIZE * i,
				&p1, NULL, NULL);

		stat_t ok2 = stat_vpage(c->rpc.vmem, RPC_STACK_BASE + BASE_PAGE_SIZE * i,
				&p2, NULL, NULL);

		assert(ok1 == OK && ok2 == OK);
		memcpy((void *)p2, (void *)p1, BASE_PAGE_SIZE);
	}
}

stat_t setup_rpc_stack(struct tcb *t)
{
	/* by default rpc stack is marked inaccessible to generate segfaults on
	 * access so as to ease stack usage tracking */
	vmflags_t flags = VM_V | VM_R | VM_W | VM_U;

	for (size_t i = 0; i < rpc_pages; ++i) {
		pm_t page = alloc_page(BASE_PAGE);
		if (!page)
			return ERR_OOMEM;

		if (map_vpage(t->rpc.vmem, page,
		              RPC_STACK_BASE + BASE_PAGE_SIZE * i,
		              flags, BASE_PAGE))
			return ERR_OOMEM;

	}

	/* we allocated a second order page for rpc stack usage */
	t->rpc_stack = RPC_STACK_BASE + order_size(MM_O1);

	/* slightly hacky maybe but we know the first pte is at RPC_STACK_BASE,
	 * which means that it must also be the leaf */
	t->arch.rpc_leaf = (struct vmem *)__find_vmem(t->rpc.vmem,
	                                              RPC_STACK_BASE,
	                                              NULL);
	/* we count downward in base pages */
	t->arch.rpc_idx = rpc_pages;
	return OK;
}

void destroy_rpc_stack(struct tcb *t)
{
	for (size_t i = 0; i < rpc_pages; ++i) {
		pm_t page = 0; enum mm_order order = BASE_PAGE;
		if (stat_vpage(t->rpc.vmem, RPC_STACK_BASE + BASE_PAGE_SIZE * i,
		               &page, &order, NULL))
			return;

		free_page(order, page);
	}
}

void reset_rpc_stack(struct tcb *t)
{
	t->rpc_stack = RPC_STACK_BASE + (BASE_PAGE_SIZE * rpc_pages);
	t->arch.rpc_idx = rpc_pages;
}

bool rpc_stack_empty(pm_t addr)
{
	return addr == RPC_STACK_BASE + (BASE_PAGE_SIZE * rpc_pages);
}


vm_t rpc_position(struct tcb *t)
{
	/** @todo we assume rpc_idx is updated on every segfault of the rpc stack */
	/** @todo hmm, technically speaking we always know that on riscv the base
	 * page size if 4096, would it be a good idea to replace BASE_PAGE_SIZE in
	 * riscv-specific code with a RISCV_BASE_PAGE_SIZE or something? */
	return RPC_STACK_BASE + (BASE_PAGE_SIZE * t->arch.rpc_idx);
}

void mark_rpc_invalid(struct tcb *t, vm_t top)
{
	struct vmem *b = t->arch.rpc_leaf;
	int top_idx = t->arch.rpc_idx;
	int bottom_idx = (top - RPC_STACK_BASE) / BASE_PAGE_SIZE;
	assert(bottom_idx < top_idx);

	while (top_idx != bottom_idx) {
		pm_t *pte = (pm_t *)&b->leaf[top_idx];
		/* make page not accessible from userspace */
		clear_bits(*pte, vp_flags(VM_U));
		top_idx--;
	}

	t->arch.rpc_idx = top_idx;
}

void mark_rpc_valid(struct tcb *t, vm_t bottom)
{
	struct vmem *b = t->arch.rpc_leaf;
	int bottom_idx = t->arch.rpc_idx;
	int top_idx = (bottom - RPC_STACK_BASE) / BASE_PAGE_SIZE;
	assert(bottom_idx <= top_idx);

	while (top_idx != bottom_idx) {
		pm_t *pte = (pm_t *)&b->leaf[bottom_idx];
		/* make page accessible from userspace */
		set_bits(*pte, vp_flags(VM_U));
		/* clear used bits */
		clear_bits(*pte, vp_flags(VM_A | VM_D));
		bottom_idx++;
	}

	t->arch.rpc_idx = bottom_idx;
}
