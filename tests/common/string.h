#ifndef KMI_TESTS_STRING_H
#define KMI_TESTS_STRING_H

#include <kmi/types.h>

char *strcpy(char * restrict dst, const char * restrict src);
char *strncpy(char * restrict dst, const char * restrict src, size_t num);
char *strncat(char * restrict dst, const char * restrict src, size_t num);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t num);
char *strchr(const char *str, int chr);
char *strtok(char * restrict str, const char * restrict delims);
char *strstr(const char *str1, const char *str2);
char *strrchr(const char *str, int chr);
char *strpbrk(const char *str1, const char *str2);
size_t strspn(const char *str1, const char *str2);
size_t strcspn(const char *str1, const char *str2);
size_t strlen(const char *str);
size_t strnlen(const char *str, size_t num);
void *memset(void *ptr, int value, size_t num);
void *memchr(const void *ptr, int val, size_t num);
void *memcpy(void * restrict dst, const void * restrict src, size_t num);
void *memmove(void *dst, const void *src, size_t num);
int memcmp(const void *ptr1, const void *ptr2, size_t num);
uintptr_t strtouintptr(const char *s);
#endif /* KMI_TESTS_STRING_H */
