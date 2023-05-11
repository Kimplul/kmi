/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file dbg_fdt.c
 * Dump fdt info to serial.
 */

#include <kmi/debug.h>
#include <libfdt.h>

#if defined(DEBUG)
/**
 * Print single character \c depth number of times.
 *
 * @param c Character to print.
 * @param depth Number of times to print character.
 */
static void __print_char(char c, int depth)
{
	/* lol, ugly but good enough for now */
	for (int i = 0; i < depth; ++i)
		dbg("%c", c);
}

/**
 * Check if ASCII character is in printable range.
 *
 * In this case, printable is anything that isn't a control character, line
 * feed, bell, etc.
 *
 * @param c Character to check.
 * @return \ref true if in printable range, \ref false otherwise.
 */
static int __printable(char c)
{
	return (c >= 32) && (c <= 126);
}

/**
 * Check if \c data and \c len bytes from it form a reasonable string.
 *
 * @param data Pointer to some binary data to check.
 * @param len Assumed length of string.
 * @return \c true if data is likely some string, \c false otherwise.
 */
static int __is_string(const void *data, int len)
{
	const char *s = data;

	/* zero length is not */
	if (len == 0)
		return 0;

	/* must terminate with zero or '\n' */
	if (s[len - 1] != '\0' && s[len - 1] != '\n')
		return 0;

	/* printable or a null byte (concatenated strings) */
	while (((*s == '\0') || __printable(*s)) && (len > 0)) {
		/*
		 * If we see a null, there are three possibilities:
		 * 1) If len == 1, it is the end of the string, printable
		 * 2) Next character also a null, not printable.
		 * 3) Next character not a null, continue to check.
		 */
		if (s[0] == '\0') {
			if (len == 1)
				return 1;
			if (s[1] == '\0')
				return 0;
		}
		s++;
		len--;
	}

	/* Not the null termination, or not done yet: not printable */
	if (*s != '\0' || (len != 0))
		return 0;

	return 1;
}

/**
 * Print value of FDT property.
 *
 * @param data Pointer to FDT property.
 * @param len Length of property.
 */
static void __print_prop_value(const void *data, int len)
{
	if (len == 0)
		return;

	dbg(" = ");

	/* heavily inspired by u-boot's fdt print */
	if (__is_string(data, len)) {
		dbg("\"");
		int i = 0;
		while (i < len) {
			if (i > 0)
				dbg("\", \"");

			dbg("%s", (const char *)data);
			i += strlen(data) + 1;
			data += strlen(data) + 1;
		}
		dbg("\"");

	} else if ((len % 4) == 0) {
		const int32_t *p = (const int32_t *)data;
		dbg("<");
		for (int i = 0; i < len / 4; ++i)
			dbg("%#08x%s", (unsigned int)fdt32_to_cpu(p[i]),
			    i < (len / 4 - 1) ? " " : "");

		dbg(">");
	} else {
		const int32_t *p = (const int32_t *)data;
		dbg("[");
		for (int i = 0; i < len / 4; ++i)
			dbg("%#02x%s", (unsigned int)fdt32_to_cpu(p[i]),
			    i < (len / 4 - 1) ? " " : "");

		dbg("]");
	}
}

void __dbg_fdt(const void *fdt, int node_offset, int depth)
{
	int node = 0;
	fdt_for_each_subnode(node, fdt, node_offset)
	{
		__print_char('\t', depth);
		dbg("%s: {\n", fdt_get_name(fdt, node, 0));

		int property = 0;
		fdt_for_each_property_offset(property, fdt, node)
		{
			int len = 0;
			const char *name = 0;
			const void *data = fdt_getprop_by_offset(fdt, property,
			                                         &name, &len);

			__print_char('\t', depth + 1);
			dbg("%s", name);

			__print_prop_value(data, len);
			dbg(";\n");
		}

		__dbg_fdt(fdt, node, depth + 1);

		__print_char('\t', depth);
		dbg("};\n\n");
	}
}
#endif
