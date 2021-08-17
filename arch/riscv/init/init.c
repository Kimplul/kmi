#include <stddef.h>
#include <apos/init.h>
#include <apos/sizes.h>
#include <pages.h>
#include <vmap.h>

struct page_ret {
	size_t left;
	size_t num;
	void *top;
};

extern void main(struct mm_pagearr_info *pageinfo);

void __init init_zero(void *ptr, size_t bytes)
{
	char *byte_ptr = ptr;

	/* Not entirely sure if it would be faster to use a larget type than
	 * char, Linux seems to have memset16/32/64 but I guess I should check
	 * the generated assembly.
	 */
	while(bytes--)
		*byte_ptr++ = 0;
}

struct page_ret __init init_pages(size_t ram_size, void *ram_top,
		size_t page_size, size_t struct_size)
{
	size_t num_pages = ram_size / page_size;
	size_t size_pages = num_pages * struct_size;

	struct page_ret pageret;
	pageret.top = ram_top - size_pages;
	pageret.num = num_pages;
	pageret.left = ram_size - (size_pages * page_size);

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
			: "rK" (root_page)
			: "memory");
}

void call_main()
{
}

void __init init(void *ram_base, void *ram_top, void *initrd, void *fdt, void *boot)
{
	size_t ram_size = ram_top - ram_base;

	struct mm_pagearr_info pagearr;
	struct page_ret pageret;

	/* create crude page status map at the top of physical RAM */
	pageret = init_pages(ram_size, ram_top,
			SZ_512G, sizeof(struct terapages));
	pagearr.tera_num = pageret.num;
	pagearr.tera_top = pageret.top;
	
	pageret = init_pages(pageret.left, pageret.top,
			SZ_1G, sizeof(struct gigapages));
	pagearr.giga_num = pageret.num;
	pagearr.giga_top = pageret.top;

	pageret = init_pages(pageret.left, pageret.top,
			SZ_2M, sizeof(struct megapages));
	pagearr.mega_num = pageret.num;
	pagearr.mega_top = pageret.top;

	pageret = init_pages(pageret.left, pageret.top,
			SZ_4K, sizeof(struct kilopages));
	pagearr.kilo_num = pageret.num;
	pagearr.kilo_top = pageret.top;

	unsigned long *root_page = (unsigned long *)
		(init_align_down((size_t)pagearr.kilo_top, SZ_4K) - SZ_4K);

	/* TODO: should init insert page data into the created arrays? */

	/* map .text.* into top part of RAM */
	root_page[511] = ((unsigned long)&main & VM_AL) | VM_V | VM_R | VM_W | VM_X;

	/* temporarily map .init.* so that our current physical address matches
	 * the virtual address */
	root_page[0] = 0 | VM_V | VM_R | VM_W | VM_X;

	/* enter virtual memory */
	init_enter_vm(root_page);

	/* enter main */
	call_main();
}
