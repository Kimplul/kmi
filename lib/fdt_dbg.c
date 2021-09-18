#include <apos/debug.h>
#include <libfdt.h>

#if defined(DEBUG)
static void __print_char(char c, int depth)
{
	/* lol, ugly but good enough for now */
	for(int i = 0; i < depth; ++i)
		dbg("%c", c);
}

static int __printable(char c)
{
	return (c >= 32) && (c <= 126);
}

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

static void __print_prop_value(const void *data, int len)
{
	if(len == 0)
		return;

	dbg(" = ");

	/* heavily inspired by u-boot's fdt print */
	if(__is_string(data, len)){
		dbg("\"");
		int i = 0;
		while(i < len){
			if(i > 0)
				dbg("\", \"");

			dbg("%s", (const char *)data);
			i += strlen(data) + 1;
			data += strlen(data) + 1;
		}
		dbg("\"");

	} else if ((len % 4) == 0) {
		const int32_t *p = (const int32_t *)data;
		dbg("<");
		for(int i = 0; i < len / 4; ++i)
			dbg("%#08x%s", fdt32_to_cpu(p[i]), i < (len / 4 - 1) ? " " : "");

		dbg(">");
	} else {
		const int32_t *p = (const int32_t *)data;
		dbg("[");
		for(int i = 0; i < len / 4; ++i)
			dbg("%#02x%s", fdt32_to_cpu(p[i]), i < (len / 4 - 1) ? " " : "");

		dbg("]");
	}
}

void __dbg_fdt(void *fdt, int node_offset, int depth)
{
	int node = 0;
	fdt_for_each_subnode(node, fdt, node_offset){
		__print_char('\t', depth);
		dbg("%s: {\n", fdt_get_name(fdt, node, 0));

		int property = 0;
		fdt_for_each_property_offset(property, fdt, node){

			int len;
			const char *name;
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
