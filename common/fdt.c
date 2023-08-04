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
