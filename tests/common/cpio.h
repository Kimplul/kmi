#include <kmi/utils.h>

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
static inline struct cpio_header *cpio_next_entry(struct cpio_header *cp)
{
	size_t blen = align_up(sizeof(struct cpio_header)
			+ convnum(cp->c_namesize, 8, 16), 4);
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
static inline struct cpio_header *cpio_find_file(const char *c,
					const char *fname,
                                        size_t fname_len)
{
	struct cpio_header *cp = (struct cpio_header *)c;
	for (; cp; cp = cpio_next_entry(cp)) {
		size_t namelen = convnum(cp->c_namesize, 8, 16);
		if (namelen == 0)
			return NULL;

		if (namelen < fname_len)
			continue;


		char *name = (char *)(cp + 1);
		if (strcmp(name, "TRAILER!!!") == 0)
			return NULL;

		if (fname[0] != '/')
			name += namelen - (fname_len + 1); /* match ending */

		if (strncmp(name, fname, fname_len) == 0)
			return cp;
	}

	printf("???\n");
	return NULL;
}
