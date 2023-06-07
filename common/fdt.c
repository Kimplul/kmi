/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file fdt.c
 * Helper functions for handling the global FDT.
 */

#include <libfdt.h>

struct cell_info get_cellinfo(const void *fdt, const int offset)
{
	return (struct cell_info){ fdt_size_cells(fdt, offset),
		                   fdt_address_cells(fdt, offset) };
}

/* how "reg" is interpreted depends on the parent node */
struct cell_info get_reginfo(const void *fdt, const char *path)
{
	const char *i = strrchr(path, '/');
	if (!i)
		return (struct cell_info){ 0, 0 };

	size_t baselen = i - path;
	if (baselen == 0)
		/* root node */
		baselen = 1;

	return get_cellinfo(fdt, fdt_path_offset_namelen(fdt, path, baselen));
}
