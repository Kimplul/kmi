#include <apos/initrd.h>
#include <apos/string.h>
#include <apos/utils.h>
#include <apos/attrs.h>
#include <libfdt.h>

/* GNU cpio, use POSIX 'newc' format */
__packed struct cpio_header {
	char c_magic[6];
	char c_ino[8];
	char c_mode[8];
	char c_uid[8];
	char c_gid[8];
	char c_nlink[8];
	char c_mtime[8];
	char c_filesize[8];
	char c_devmajor[8];
	char c_devminor[8];
	char c_rdevmajor[8];
	char c_rdevminor[8];
	char c_namesize[8];
	char c_check[8];
};

static struct cpio_header *next_entry(struct cpio_header *cp)
{
	size_t blen = align_up(sizeof(struct cpio_header) + convnum(cp->c_namesize, 8, 16), 4);
	size_t tlen = align_up(convnum(cp->c_filesize, 8, 16), 4);

	return (struct cpio_header *)(((char *)cp) + blen + tlen);
}

static struct cpio_header *find_file(char *c, char* fname, size_t fname_len)
{
	struct cpio_header *cp = (struct cpio_header *)c;
	for(; cp; cp = next_entry(cp)){
		size_t namelen = convnum(cp->c_namesize, 8, 16);
		if(namelen == 0)
			return 0;

		if(namelen < fname_len)
			continue;

		char *name = (char *)(cp + 1);
		if(fname[0] != '/')
			name += namelen - (fname_len + 1); /* match ending */

		if(strncmp(name, fname, fname_len) == 0)
			return cp;
	}

	return 0;
}

pm_t get_initrdtop(void *fdt)
{
	int chosen_offset = fdt_path_offset(fdt, "/chosen");
	struct cell_info_t ci = get_cellinfo(fdt, chosen_offset);

	void *initrd_end_ptr = (void *)fdt_getprop(fdt, chosen_offset,
			"linux,initrd-end", NULL);

	return (pm_t)fdt_load_int_ptr(ci.addr_cells, initrd_end_ptr);
}

pm_t get_initrdbase(void *fdt)
{
	int chosen_offset = fdt_path_offset(fdt, "/chosen");
	struct cell_info_t ci = get_cellinfo(fdt, chosen_offset);

	void *initrd_base_ptr = (void *)fdt_getprop(fdt, chosen_offset,
			"linux,initrd-start", NULL);

	return (pm_t)fdt_load_int_ptr(ci.addr_cells, initrd_base_ptr);
}

static char init_n[] = "init";
static size_t init_nlen = ARRAY_SIZE(init_n) - 1; /* ignore trailing NULL */

size_t get_init_size(void *fdt)
{
	char *c = (char *)get_initrdbase(fdt);
	struct cpio_header *cp = find_file(c, init_n, init_nlen);
	return convnum(cp->c_filesize, 8, 16);
}

void move_init(void *fdt, void *target)
{
	char *c = (char *)get_initrdbase(fdt);

	struct cpio_header *cp = find_file(c, init_n, init_nlen);
	size_t name_len = convnum(cp->c_namesize, 8, 16);
	size_t file_len = convnum(cp->c_filesize, 8, 16);

	char *fp = (char *)cp;
	fp += align_up(sizeof(struct cpio_header) + name_len, 4);

	memmove(target, fp, file_len);
}
