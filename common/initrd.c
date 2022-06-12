/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file initrd.c
 * Handle initrd, implement cpio with newc format.
 */

#include <apos/initrd.h>
#include <apos/vmem.h>
#include <apos/string.h>
#include <apos/utils.h>
#include <apos/attrs.h>
#include <libfdt.h>

/** GNU cpio, POSIX 'newc' format header. */
struct __packed cpio_header {
	/** Magic bytes. */
	char c_magic[6];

	/** File inode. */
	char c_ino[8];

	/** File type and permissions. */
	char c_mode[8];

	/** User ID. */
	char c_uid[8];

	/** Group ID. */
	char c_gid[8];

	/** Number of links to this file. */
	char c_nlink[8];

	/** Modification time. */
	char c_mtime[8];

	/** Size of file. */
	char c_filesize[8];

	/** Device major. */
	char c_devmajor[8];

	/** Device minor. */
	char c_devminor[8];

	/** Block device major. */
	char c_rdevmajor[8];

	/** Block device minor. */
	char c_rdevminor[8];

	/** Length of filename. */
	char c_namesize[8];

	/** CRC check. */
	char c_check[8];
};

/**
 * Get next file in archive.
 * Does not check for out of bounds.
 *
 * @param cp Pointer to current file header.
 * @return Pointer to next file header.
 */
static struct cpio_header *__next_entry(struct cpio_header *cp)
{
	size_t blen = align_up(
		sizeof(struct cpio_header) + convnum(cp->c_namesize, 8, 16), 4);
	size_t tlen = align_up(convnum(cp->c_filesize, 8, 16), 4);

	return (struct cpio_header *)(((char *)cp) + blen + tlen);
}

/**
 * Get file with name in archive.
 *
 * @param c Pointer to initrd.
 * @param fname Filename to look for.
 * @param fname_len Length of filename.
 * @return Pointer to corresponding file header if found, \c NULL otherwise.
 */
static struct cpio_header *__find_file(const char *c, const char *fname,
                                       size_t fname_len)
{
	struct cpio_header *cp = (struct cpio_header *)c;
	for (; cp; cp = __next_entry(cp)) {
		size_t namelen = convnum(cp->c_namesize, 8, 16);
		if (namelen == 0)
			return NULL;

		if (namelen < fname_len)
			continue;

		char *name = (char *)(cp + 1);
		if (fname[0] != '/')
			name += namelen - (fname_len + 1); /* match ending */

		if (strncmp(name, fname, fname_len) == 0)
			return cp;
	}

	return NULL;
}

/** Name of \c init program. */
static char init_n[] = "init";

/** Length of \c init name. */
static size_t init_nlen = ARRAY_SIZE(init_n) - 1; /* ignore trailing NULL */

pm_t get_initrdtop(const void *fdt)
{
	int chosen_offset = fdt_path_offset(fdt, "/chosen");
	struct cell_info ci = get_cellinfo(fdt, chosen_offset);

	void *initrd_end_ptr = (void *)fdt_getprop(fdt, chosen_offset,
	                                           "linux,initrd-end", NULL);

	/* fdt is only aware of physical memory pointers */
	return (pm_t)__va(fdt_load_int_ptr(ci.addr_cells, initrd_end_ptr));
}

pm_t get_initrdbase(const void *fdt)
{
	const int chosen_offset = fdt_path_offset(fdt, "/chosen");
	const struct cell_info ci = get_cellinfo(fdt, chosen_offset);

	void *initrd_base_ptr = (void *)fdt_getprop(fdt, chosen_offset,
	                                            "linux,initrd-start", NULL);

	return (pm_t)__va(fdt_load_int_ptr(ci.addr_cells, initrd_base_ptr));
}


size_t get_init_size(const void *fdt)
{
	char *c = (char *)get_initrdbase(fdt);
	struct cpio_header *cp = __find_file(c, init_n, init_nlen);
	return convnum(cp->c_filesize, 8, 16);
}

vm_t get_init_base(const void *fdt)
{
	char *c = (char *)get_initrdbase(fdt);
	struct cpio_header *cp = __find_file(c, init_n, init_nlen);
	size_t name_len = convnum(cp->c_namesize, 8, 16);
	return ((vm_t)cp) + align_up(sizeof(struct cpio_header) + name_len, 4);
}

stat_t move_init(const void *fdt, void *target)
{
	const char *c = (const char *)get_initrdbase(fdt);

	const struct cpio_header *cp = __find_file(c, init_n, init_nlen);
	size_t name_len = convnum(cp->c_namesize, 8, 16);
	size_t file_len = convnum(cp->c_filesize, 8, 16);

	char *fp = (char *)cp;
	fp += align_up(sizeof(struct cpio_header) + name_len, 4);

	memmove(target, fp, file_len);
	return OK;
}
