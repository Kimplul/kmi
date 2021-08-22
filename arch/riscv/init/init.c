#include <apos/init.h>
#include <apos/sizes.h>
#include <apos/types.h>
#include <pages.h>
#include <csr.h>
#include <vmap.h>

struct page_ret {
	size_t left;
	size_t num;
	size_t top;
};

extern void main(struct mm_pagearr_info *pageinfo);

void __init init_zero(size_t ptr, size_t bytes)
{
	char *byte_ptr = (char *)ptr;

	/* Not entirely sure if it would be faster to use a larget type than
	 * char, Linux seems to have memset16/32/64 but I guess I should check
	 * the generated assembly.
	 */
	while(bytes--)
		*byte_ptr++ = 0;
}

struct page_ret __init init_pages(size_t ram_size, size_t ram_top,
		size_t page_size, size_t struct_size)
{
	size_t num_pages = ram_size / page_size;
	size_t size_pages = num_pages * struct_size;

	struct page_ret pageret;
	pageret.top = ram_top - size_pages;
	pageret.num = num_pages;
	pageret.left = ram_size - (num_pages * page_size);

	init_zero(pageret.top, size_pages);

	return pageret;
};

size_t __init init_align_down(size_t num, size_t a)
{
	size_t rem = num % a;
	return num - rem;
}

size_t __init init_align_up(size_t num, size_t a)
{
	return init_align_down(num, a) + a;
}

void __init init_enter_vm(unsigned long *root_page)
{
	asm volatile ("csrw 0x180, %0\n" "sfence.vma\n"
			:
			: "rK" (SATP_MODE_39 | ((unsigned long)root_page >> 12))
			: "memory");
}

void __init call_main()
{
	while(1){};
}

void __init init(void *ram_base, void *ram_top, void *initrd, void *fdt, void *boot)
{
	size_t ram_size = ram_top - ram_base;

	struct mm_pagearr_info pagearr;
	struct page_ret pageret;

	/* create crude page status map at the top of physical RAM */
	pageret = init_pages(ram_size, (size_t)ram_top,
			SZ_512G, sizeof(struct terapages));
	pagearr.tera_top = (struct terapages *)pageret.top;
	pagearr.tera_num = pageret.num;
	
	pageret = init_pages(pageret.left, pageret.top,
			SZ_1G, sizeof(struct gigapages));
	pagearr.giga_top = (struct gigapages *)pageret.top;
	pagearr.giga_num = pageret.num;

	pageret = init_pages(pageret.left, pageret.top,
			SZ_2M, sizeof(struct megapages));
	pagearr.mega_top = (struct megapages *)pageret.top;
	pagearr.mega_num = pageret.num;

	pageret = init_pages(pageret.left, pageret.top,
			SZ_4K, sizeof(struct kilopages));
	pagearr.kilo_top = (struct kilopages *)pageret.top;
	pagearr.kilo_num = pageret.num;

	unsigned long *root_page = (unsigned long *)
		(init_align_down((size_t)pagearr.kilo_top, SZ_4K) - SZ_4K);

	unsigned long *mid_page = (unsigned long)root_page - SZ_4K;
	unsigned long *bot_page = (unsigned long)mid_page - SZ_4K;

	/* TODO: should init insert page data into the created arrays? */
	unsigned long root_addr = (((unsigned long)boot >> 12) >> 18) & 0x1ff;
	unsigned long mid_addr = (((unsigned long)boot >> 12) >> 9) & 0x1ff;
	unsigned long bot_addr = (((unsigned long)boot >> 12) >> 0) & 0x1ff;

	root_page[root_addr] = (((unsigned long)mid_page & ~0x3ff) >> 2) | VM_V;
	mid_page[mid_addr] = ((unsigned long)bot_page & ~0x3ff) >> 2 | VM_V;
	bot_page[bot_addr] = (((unsigned long)boot & ~0x3ff) >> 2) | VM_V | VM_R | VM_W | VM_X;

	//root_page[0] = (((unsigned long)ram_base & ~0x3ff) >> 2) | VM_V | VM_R | VM_W | VM_X;

	/* enter virtual memory */
	init_enter_vm(root_page);

	/* enter main */
	call_main();
}
