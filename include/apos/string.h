#ifndef APOS_STRING_H
#define APOS_STRING_H

#include <apos/types.h>

/* follow C library functions, drop location and error functions */

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t num);

char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t num);

int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t num);

char *strchr(const char *str, int chr);
char *strtok(char *str, const char *delims);
char *strstr(const char *str1, const char *str2);

char *strrchr(const char *str, int chr);
char *strpbrk(const char *str1, const char *str2);

size_t strspn(const char *str1, const char *str2);
size_t strcspn(const char *str1, const char *str2);

size_t strlen(const char *str);
/* only addition on top of libc */
size_t strnlen(const char *str, size_t num);

void *memset(void *ptr, int value, size_t num);
void *memchr(const void *ptr, int val, size_t num);
void *memcpy(void *dst, const void *src, size_t num);
void *memmove(void *dst, const void *src, size_t num);

int memcmp(const void *ptr1, const void *ptr2, size_t num);

/* Honorable mentions:
 *
 * char *strerror(int err);
 * int strcoll(const char *str1, const char *str2);
 * int strxfrm(char *dest, const char *src, size_t num);
 *
 */

/* provide macros for builtin functions. The compiler is always allowed to
 * replace a __builtin_* with a regular call to the function, which is why we
 * still need to define the functions.
 *
 * Does mean we can't take the address of any of these, but I suppose that's not
 * an issue.
 */

#define memcpy(dst, src, num)	__builtin_memcpy(dst, src, num)
#define memmove(dst, src, num)	__builtin_memmove(dst, src, num)
#define strcpy(dst, src)	__builtin_strcpy(dst, src)
#define strncpy(dst, src, num)	__builtin_strncpy(dst, src, num)

#define strcat(dst, src)	__builtin_strcat(dst, src)
#define strncat(dst, src, num)	__builtin_strncat(dst, src, num)

#define memcmp(p1, p2, num)	__builtin_memcmp(p1, p2, num)
#define strcmp(s1, s2)		__builtin_strcmp(s1, s2)
#define strncmp(s1, s2, num)	__builtin_strncmp(s1, s2, num)

#define memchr(ptr, val, num)	__builtin_memchr(ptr, val, num)
#define strchr(str, chr)	__builtin_strchr(str, chr)
#define strcspn(s1, s2)		__builtin_strcspn(s1, s2)
#define strpbrk(s1, s2)		__builtin_strpbrk(s1, s2)
#define strrchr(s1, s2)		__builtin_strrchr(s1, s2)
#define strspn(s1, s2)		__builtin_strspn(s1, s2)
#define strstr(s1, s2)		__builtin_strstr(s1, s2)
#define strtok(s1, s2)		__builtin_strtok(s1, s2)

#define memset(p, v, n)		__builtin_memset(p, v, n)
#define strlen(s)		__builtin_strlen(s)

#endif /* APOS_STRING_H */
