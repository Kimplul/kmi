/* copied straight from the kernel */
#include <common/string.h>
#include <kmi/utils.h>
#include <kmi/types.h>

char *strcpy(char * restrict dst, const char * restrict src)
{
	const char *s1 = src;
	char *s2 = dst;

	while (*s1)
		*(s2++) = *(s1++);

	return dst;
}

char *strncpy(char * restrict dst, const char * restrict src, size_t num)
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

char *strcat(char * restrict dst, const char * restrict src)
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

char *strncat(char * restrict dst, const char * restrict src, size_t num)
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

int strcmp(const char *str1, const char *str2)
{
	const char *s1 = (const char *)str1;
	const char *s2 = (const char *)str2;

	while ((*(s1++) == *(s2++)) && *s1 && *s2)
		;

	return (int)(s1[-1] - s2[-1]);
}

int strncmp(const char *str1, const char *str2, size_t num)
{
	const char *s1 = (const char *)str1;
	const char *s2 = (const char *)str2;

	while ((*(s1++) == *(s2++)) && *s1 && *s2 && --num)
		;

	return (int)(s1[-1] - s2[-1]);
}

char *strchr(const char *str, int chr)
{
	const char *s1 = str;
	ssize_t num = strlen(s1);

	while (num-- && *(s1++) != chr)
		;

	if (num < 0)
		return 0;

	return (char *)(s1 - 1);
}

char *strtok(char * restrict str, const char * restrict delims)
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
char *strstr(const char *str1, const char *str2)
{
	/* boyer-moore-horspool */
	char table[256] = { 0 };
	size_t sl = strlen(str1);
	size_t pl = strlen(str2);

	const unsigned char *s1 = (const unsigned char *)str1;
	const unsigned char *haystack = (const unsigned char *)s1;
	const unsigned char *needle = (const unsigned char *)str2;

	for (size_t i = 0; i < 256; ++i)
		table[i] = pl;

	/* generate deltas */
	for (size_t i = 0; i < pl - 1; ++i)
		table[needle[i]] = pl - i - 1;

	size_t skip = 0;
	while (sl - skip >= pl) {
		s1 = &haystack[skip];

		if (!memcmp(s1, needle, pl))
			return (char *)s1;

		skip += table[haystack[skip + pl - 1]];
	}

	return 0;
}

char *strrchr(const char *str, int chr)
{
	ssize_t num = strlen(str);
	const char *s1 = (str + num) - 1;

	while (num-- && *(s1--) != chr)
		;

	if (num < 0)
		return 0;

	return (char *)(s1 + 1);
}

char *strpbrk(const char *str1, const char *str2)
{
	size_t i = strcspn(str1, str2);

	if (!i)
		return 0;

	return (char *)(str1 + i);
}

size_t strcspn(const char *str1, const char *str2)
{
	char table[256] = { 0 };
	const unsigned char *s1 = (const unsigned char *)str1;
	const unsigned char *s2 = (const unsigned char *)s1;
	const unsigned char *t1 = (const unsigned char *)str2;

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

size_t strspn(const char *str1, const char *str2)
{
	char table[256] = { 0 };
	const unsigned char *s1 = (const unsigned char *)str1;
	const unsigned char *s2 = (const unsigned char *)s1;
	const unsigned char *t1 = (const unsigned char *)str2;

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

size_t strlen(const char *str)
{
	const char *s1 = str;
	while (*(s1++))
		;

	/* the loop overshoots by one */
	return (size_t)(s1 - str) - 1;
}

/* not a macro */
size_t strnlen(const char *str, size_t num)
{
	const char *s1 = str;
	while (num-- && *(s1++))
		;

	return (size_t)(s1 - str) - 1;
}

void *memchr(const void *ptr, int val, size_t num)
{
	const char *p1 = (char *)ptr;
	ssize_t n = num;
	char c = (char)val;

	while (n-- && *(p1++) != c)
		;

	if (n < 0)
		return 0;

	return (void *)(p1 - 1);
}

/** @defgroup group1 Optimized specializations
 *
 * The two most used memory operations get their special optimized versions for
 * aligned manipulations, which is what the vast majority of our huge operations
 * like page zeroing are.
 *
 * @{
 */

/** How many longs per iteration we want to process. An address and size must be
 * aligned to MAGIC_NUMBER * sizeof(long) bytes. */
#define MAGIC_NUMBER 8

/**
 * Optimized version of memcpy() for big regions.
 * All parameters must be aligned to MAGIC_NUMBER * sizeof(long).
 *
 * @param dst Destination to copy to.
 * @param src Source to copy from.
 * @param num Number of bytes to copy.
 * @return dst
 */
static inline void *__aligned_memcpy(long *restrict dst,
                                     const long *restrict src, size_t num)
{
	size_t count = num / sizeof(long);
	for (size_t i = 0; i < count; i += MAGIC_NUMBER) {
		dst[i + 0] = src[i + 0];
		dst[i + 1] = src[i + 1];
		dst[i + 2] = src[i + 2];
		dst[i + 3] = src[i + 3];
		dst[i + 4] = src[i + 4];
		dst[i + 5] = src[i + 5];
		dst[i + 6] = src[i + 6];
		dst[i + 7] = src[i + 7];
		/* would be kind of cool if there was some kind of forced unrolling,
		 * as we need to do MAGIC_NUMBER of steps, preferably even with
		 * optimizations disabled. For now I guess just be careful that
		 * MAGIC_NUMBER matches the number of steps within loop */
	}

	return dst;
}

/**
 * Optimized version of memset() for big regions.
 * \p ptr and \p num must be aligned to MAGIC_NUMBER * sizeof(long).
 *
 * @param ptr Destination to write to.
 * @param value Value (converted to unsigned char) to write.
 * @param num Number of bytes to write.
 * @return ptr
 */
static inline void *__aligned_memset(long *ptr, int value, size_t num)
{
	long bits = 0;
	for (size_t i = 0; i < sizeof(long) / sizeof(unsigned char); ++i)
		bits |= ((unsigned char)value) << i * CHAR_BIT;

	size_t count = num / sizeof(long);
	for (size_t i = 0; i < count; i += MAGIC_NUMBER) {
		ptr[i + 0] = bits;
		ptr[i + 1] = bits;
		ptr[i + 2] = bits;
		ptr[i + 3] = bits;
		ptr[i + 4] = bits;
		ptr[i + 5] = bits;
		ptr[i + 6] = bits;
		ptr[i + 7] = bits;
	}

	return ptr;
}

/** @} */

void *memcpy(void * restrict dst, const void * restrict src,
                           size_t num)
{

	if (is_aligned((uintptr_t)dst, sizeof(long) * MAGIC_NUMBER)
	    && is_aligned((uintptr_t)src, sizeof(long) * MAGIC_NUMBER)
	    && is_aligned(num, sizeof(long) * MAGIC_NUMBER))
		return __aligned_memcpy(dst, src, num);

	const char *m1 = (const char *)src;
	char *m2 = (char *)dst;

	while (num--)
		*(m2++) = *(m1++);

	return dst;
}

void *memset(void *ptr, int value, size_t num)
{
	if (is_aligned((uintptr_t)ptr, sizeof(long) * MAGIC_NUMBER)
	    && is_aligned(num, sizeof(long) * MAGIC_NUMBER))
		return __aligned_memset(ptr, value, num);

	char *p = ptr;
	char c = value;

	while (num--)
		*(p++) = c;

	return ptr;
}


void *memmove(void *dst, const void *src, size_t num)
{
	const char *m1 = (const char *)src;
	char *m2 = (char *)dst;

	m1 += num;
	m2 += num;

	/* doesn't really take into account aliasing yet */
	while (num--)
		*(--m2) = *(--m1);

	return dst;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
	const char *p1 = (const char *)ptr1;
	const char *p2 = (const char *)ptr2;

	while ((*(p1++) == *(p2++)) && --num)
		;

	return (int)(p1[-1] - p2[-1]);
}

/**
 * Convert ASCII hex character to integer.
 * Allows both upper- and lowercase letters.
 *
 * @param c Character to convert.
 * @return Corresponding integer value. That is, '1' => 1, '2' => 2, etc.
 * \c -1 if conversion failed.
 */
static int __hexval(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';

	if (c >= 'a' && c <= 'f')
		return 10 + c - 'a';

	if (c >= 'A' && c <= 'F')
		return 10 + c - 'A';

	return -1;
}

/**
 * Convert string assumed to represent hex
 * value to corresponding pointer.
 *
 * @param s String to convert to value.
 * @return Corresponding pointer value.
 */
static uintptr_t __hexuintptr(const char *s)
{
	uintptr_t res = 0;
	int val = 0;
	while ((val = __hexval(*(s++))) != -1) {
		res *= 16;
		res += val;
	}

	return res;
}

/**
 * Convert ASCII decimal character to integer.
 *
 * @param c Character to convert.
 * @return Corresponding integer value. That is, '1' => 1, '2' => 2, etc.
 * \c -1 if conversion failed.
 */
static int __decval(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';

	return -1;
}

/**
 * Convert string assumed to represent decimal
 * value to corresponding pointer.
 *
 * @param s String to convert to value.
 * @return Corresponding pointer value.
 */
static uintptr_t __decuintptr(const char *s)
{
	uintptr_t res = 0;
	int val = 0;
	while ((val = __decval(*(s++))) != -1) {
		res *= 10;
		res += val;
	}

	return res;
}

static int __octval(char c)
{
	if (c >= '0' && c <= '7')
		return c - '0';

	return -1;
}

static uintptr_t __octuintptr(const char *s)
{
	uintptr_t res = 0;
	int val = 0;
	while ((val = __octval(*(s++))) != -1) {
		res *= 8;
		res += val;
	}

	return res;
}

uintptr_t strtouintptr(const char *s)
{
	if (!s)
		return 0;

	if (s[0] == 0)
		return 0;

	if (s[0] == '0') {
		if (s[1] == 0)
			return 0;

		if (s[1] == 'x' || s[1] == 'X')
			return __hexuintptr(s + 2);

		return __octuintptr(s + 1);
	}

	if (s[0] == '-')
		return -__decuintptr(s + 1);

	if (s[0] == '+')
		return __decuintptr(s + 1);

	return __decuintptr(s);
}
