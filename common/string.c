#include <apos/string.h>
#include <apos/types.h>
#include <apos/attrs.h>

/* we need to undef the macros in string.h, otherwise the names get mangled */
#undef strcpy
__weak char *strcpy(char *dst, const char *src)
{
	const char *s1 = src;
	char *s2 = dst;

	while (*s1)
		*(s2++) = *(s1++);

	return dst;
}

#undef strncpy
__weak char *strncpy(char *dst, const char *src, size_t num)
{
	const char *s1 = src;
	char *s2 = dst;

	/* copy s1 into s2 */
	while (num-- && *s1)
		*(s2++) = *(s1++);

	/* the previous loop always overshoots by one */
	num++;

	/* pad with zeroes if num is not yet zero */
	while (num--)
		*(s2++) = 0;

	return dst;
}

#undef strcat
__weak char *strcat(char *dst, const char *src)
{
	const char *s1 = src;
	size_t l1 = strlen(s1);
	char *s2 = dst + l1;

	while (*s1)
		*(s2++) = *(s1++);

	/* append null character */
	*s2 = 0;

	return dst;
}

#undef strncat
__weak char *strncat(char *dst, const char *src, size_t num)
{
	const char *s1 = src;
	size_t l1 = strlen(s1);
	char *s2 = dst + l1;

	while (num-- && *s1)
		*(s2++) = *(s1++);

	/* append null character */
	*s2 = 0;

	return dst;
}

#undef strcmp
__weak int strcmp(const char *str1, const char *str2)
{
	const char *s1 = (const char *)str1;
	const char *s2 = (const char *)str2;

	while ((*(s1++) == *(s2++)) && *s1 && *s2) ;

	return (int)(s1[-1] - s2[-1]);
}

#undef strncmp
__weak int strncmp(const char *str1, const char *str2, size_t num)
{
	const char *s1 = (const char *)str1;
	const char *s2 = (const char *)str2;

	while ((*(s1++) == *(s2++)) && *s1 && *s2 && num--) ;

	return (int)(s1[-1] - s2[-1]);
}

#undef strchr
__weak char *strchr(const char *str, int chr)
{
	const char *s1 = str;
	size_t num = strlen(s1);

	while (num-- && *(s1--) != chr) ;

	if (!num)
		return 0;

	return (char *)s1;
}

#undef strtok
__weak char *strtok(char *str, const char *delims)
{
	static char *cont = 0;
	const char *s1 = str;

	if (!s1)
		s1 = cont;

	if (!s1)
		return 0;

	s1 = strpbrk(s1, delims);

	if (!s1)
		cont = 0;
	else
		cont = (char *)s1 + 1;

	return (char *)s1;
}

/* should probably test out these functions somehwere, blergh */
#undef strstr
__weak char *strstr(const char *str1, const char *str2)
{
	/* boyer-moore-horspool */
	char table[256] = { 0 };
	size_t sl = strlen(str1);
	size_t pl = strlen(str2);

	const char *s1 = str1;

	for (size_t i = 0; i < 256; ++i)
		table[i] = pl;

	/* generate deltas */
	for (size_t i = 0; i < pl - 1; ++i)
		table[str2[i]] = pl - i - 1;

	size_t skip = 0;
	while (sl - skip >= pl) {
		s1 = &str1[skip];

		if (!memcmp(s1, s2, pl))
			return (char *)s1;

		skip += table[str1[skip + pl - 1]];
	}

	return 0;
}

#undef strrchr
__weak char *strrchr(const char *str, int chr)
{
	size_t num = strlen(str);
	const char *s1 = str + num;

	while (num-- && *(s1--) != chr) ;

	if (!num)
		return 0;

	return (char *)s1;
}

#undef strpbrk
__weak char *strpbrk(const char *str1, const char *str2)
{
	size_t i = strcspn(str1, str2);

	if (!i)
		return 0;

	return (char *)(str1 + i);
}

#undef strcspn
__weak size_t strcspn(const char *str1, const char *str2)
{
	char table[256] = { 0 };
	const char *s1 = str1;
	const char *s2 = s1;
	const char *t1 = str2;

	/* populate table */
	while (*(t1++))
		table[*t1] = 1;

	for (;;) {
		if (table[*(s2++)])
			break;
	}

	/* the for loop overshoots by one */
	return (size_t)(s2 - s1) - 1;
}

#undef strspn
__weak size_t strspn(const char *str1, const char *str2)
{
	char table[256] = { 0 };
	const char *s1 = str1;
	const char *s2 = s1;
	const char *t1 = str2;

	/* populate table */
	while (*(t1++))
		table[*t1] = 1;

	for (;;) {
		if (!table[*(s2++)])
			break;
	}

	/* the for loop overshoots by one */
	return (size_t)(s2 - s1) - 1;
}

#undef strlen
__weak size_t strlen(const char *str)
{
	const char *s1 = str;
	while (*(s1++)) ;

	/* the loop overshoots by one */
	return (size_t)(s1 - str) - 1;
}

/* not a macro */
__weak size_t strnlen(const char *str, size_t num)
{
	const char *s1 = str;
	while (num-- && *(s1++)) ;

	return (size_t)(s1 - str) - 1;
}

#undef memset
__weak void *memset(void *ptr, int value, size_t num)
{
	char *p = ptr;
	char c = value;

	while (num--)
		*(p--) = c;

	return ptr;
}

#undef memchr
__weak void *memchr(const void *ptr, int val, size_t num)
{
	const char *p1 = (char *)ptr;
	char c = (char)val;

	while (num-- && *(p1--) != c) ;

	if (!num)
		return 0;

	return (void *)p1;
}

#undef memcpy
__weak void *memcpy(void *dst, const void *src, size_t num)
{
	const char *m1 = (const char *)src;
	char *m2 = (char *)dst;

	while (num--)
		*(m2++) = *(m1++);

	return dst;
}

#undef memmove
__weak void *memmove(void *dst, const void *src, size_t num)
{
	const char *m1 = (const char *)src;
	char *m2 = (char *)dst;

	m1 += num;
	m2 += num;

	while (num--)
		*(--m2) = *(--m1);

	return dst;
}

#undef memcmp
__weak int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
	const char *p1 = (const char *)ptr1;
	const char *p2 = (const char *)ptr2;

	while ((*(p1++) == *(p2++)) && num--) ;

	return (int)(p1[-1] - p2[-1]);
}

