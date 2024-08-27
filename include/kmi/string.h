/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_STRING_H
#define KMI_STRING_H

/**
 * @file string.h
 * String handling, similar to cstdlib's string.h.
 */

#include <kmi/types.h>
#include <kmi/builtin.h>

/* follow C library functions, drop location and error functions */

/**
 * Copy \c NULL -terminated string.
 * \c dst may not overlap \c src.
 *
 * @param dst Destination of copy.
 * @param src Source of copy.
 * @return \c dst.
 */
char *strcpy(char * restrict dst, const char * restrict src);

/**
 * Copy \c NULL -terminated string, or first \c num characters.
 * \c dst may not overlap \c src.
 *
 * @param dst Destination of copy.
 * @param src Source of copy.
 * @param num Maximum number of bytes to copy.
 * @return \c dst.
 */
char *strncpy(char * restrict dst, const char * restrict src, size_t num);

/**
 * Concatenate \c NULL -terminated strings, placing \c src after \c dst.
 * \c dst may not overlap \c src.
 *
 * @param dst Destination of concatenation.
 * @param src String to append to \c dst.
 * @return \c dst.
 */
char *strcat(char * restrict dst, const char * restrict src);

/**
 * Concatenate \c NULL -terminated strings, max \c num characters, placing \c
 * src after \c dst.
 * \c dst may not overlap \c src.
 *
 * @param dst Destination of concatenation.
 * @param src String to append to \c dst.
 * @param num Max number of characters to concatenate.
 * @return \c dst.
 */
char *strncat(char * restrict dst, const char * restrict src, size_t num);

/**
 * Compare two \c NULL -terminated strings.
 *
 * @param str1 First string to compare.
 * @param str2 Second string to compare.
 * @return \c 0 if strings are equal, else \code str1[i] - str2[i] \endcode for
 * first differing character.
 */
int strcmp(const char *str1, const char *str2);

/**
 * Compare two \c NULL -terminated strings, max \c num characters.
 *
 * @param str1 First string to compare.
 * @param str2 Second string to compare.
 * @param num Maximum number of characters to compare.
 * @return \c 0 if strings are equal, else \code str[i] - str2[i] \endcode for
 * first differint character.
 */
int strncmp(const char *str1, const char *str2, size_t num);

/**
 * Find first character occurence of \c chr in \c str.
 *
 * @param str String to look in.
 * @param chr Character to look for.
 * @return Pointer to first \c chr in \c str, else \c 0.
 */
char *strchr(const char *str, int chr);

/**
 * Tokenize string at delimiters. Each stop character is replaced with a \c
 * NULL. Each following call after the initial found should be a \c NULL. Note
 * that this kernel's implementation is not thread safe.
 *
 * @param str String to look in.
 * @param delims String of characters to stop on.
 * @return Start of found token, \c NULL otherwise.
 */
char *strtok(char * restrict str, const char * restrict delims);

/**
 * Find first occurence of \c str2 in \c str1.
 *
 * @param str1 String to look in.
 * @param str2 String to look for.
 * @return Pointer to start of first occurence, \c NULL otherwise.
 */
char *strstr(const char *str1, const char *str2);

/**
 * Find last occurence of \c chr in \c str.
 *
 * @param str String to look in.
 * @param chr Character to look for.
 * @return Pointer to last occurence.
 */
char *strrchr(const char *str, int chr);

/**
 * Find first occurence in \c str1 of any of characters in \c str2.
 *
 * @param str1 String to look in.
 * @param str2 String of characters to look for.
 * @return Pointer to first occurence.
 */
char *strpbrk(const char *str1, const char *str2);

/**
 * Get span of characters in \c str2 in \c str1.
 *
 * @param str1 String to look in.
 * @param str2 String of characters to look for.
 * @return Number of characters at start of \c str1 that are also in \c str2.
 */
size_t strspn(const char *str1, const char *str2);

/**
 * Get span of characters not in \c str2 in \c str1.
 *
 * @param str1 String to look in.
 * @param str2 String of characters to look out for.
 * @return Number of characters at start of \c str1 that are not in \c str2.
 */
size_t strcspn(const char *str1, const char *str2);

/**
 * Length of \c NULL-terminated string.
 *
 * @param str String.
 * @return Length of \c str.
 */
size_t strlen(const char *str);
/**
 * Length of \c NULL-terminated string, max \c num.
 * Only addition on top of functions found in libc.
 *
 * @param str String.
 * @param num Max number of characters to count.
 * @return Length of \c str or \c num.
 */
size_t strnlen(const char *str, size_t num);

/**
 * Initialize memory to some value.
 *
 * @param ptr Pointer to memory to initialize.
 * @param value Value to initialize to. \note Will be truncated to byte.
 * @param num Number of bytes to initialize.
 * @return \c ptr.
 */
void *memset(void *ptr, int value, size_t num);

/**
 * Look for value in memory.
 *
 * @param ptr Pointer to memory to look in.
 * @param val Value to look for. \note Will be truncated to byte.
 * @param num Number of bytes to look.
 * @return Pointer to first occurence of \c val.
 */
void *memchr(const void *ptr, int val, size_t num);

/**
 * Copy memory byte for byte.
 * \p dst may not overlap \p src.
 *
 * @param dst Destination of copy.
 * @param src Source of copy.
 * @param num Number of bytes to copy.
 * @return \p dst.
 */
void *memcpy(void * restrict dst, const void * restrict src, size_t num);

/**
 * Move memory byte by byte.
 * \note Essentially identical to \ref memcpy(), but the memory regions may
 * overlap.
 *
 * @param dst Destination of move.
 * @param src Source of move.
 * @param num Number of bytes to move.
 * @return \c dst.
 */
void *memmove(void *dst, const void *src, size_t num);

/**
 * Compare memory byte by byte.
 *
 * @param ptr1 Memory to compare 1.
 * @param ptr2 Memory to compare 2.
 * @param num Bytes to compare.
 * @return \c 0 when equal, else
 * \code (unsigned char *)ptr1[i] - (unsigned char *)ptr2[i] \endcode
 * at first differing byte.
 */
int memcmp(const void *ptr1, const void *ptr2, size_t num);

/**
 * Try to convert string \p s into a corresponding
 * \c uintptr_t. Handles decimal, octal and hex,
 * assuming hex starts with \c 0x or \c 0X and octal with
 * \c 0, otherwise assumes decimal. Allows \c + and \c - in
 * decimal.
 *
 * Note that u-boot likes to skip leading \c 0x when using hex,
 * so if you're passing u-boot variables make sure to check prefixes.
 *
 * @param s String to convert to pointer.
 * @return Corresponding pointer value.
 */
uintptr_t strtouintptr(const char *s);

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

#if __has_builtin(__builtin_memcpy)
#define memcpy(dst, src, num) __builtin_memcpy(dst, src, num)
#endif

#if __has_builtin(__builtin_memmove)
#define memmove(dst, src, num) __builtin_memmove(dst, src, num)
#endif

#if __has_builtin(__builtin_strcpy)
#define strcpy(dst, src) __builtin_strcpy(dst, src)
#endif

#if __has_builtin(__builtin_strncpy)
#define strncpy(dst, src, num) __builtin_strncpy(dst, src, num)
#endif

#if __has_builtin(__builtin_strcat)
#define strcat(dst, src) __builtin_strcat(dst, src)
#endif

#if __has_builtin(__builtin_strncat)
#define strncat(dst, src, num) __builtin_strncat(dst, src, num)
#endif

#if __has_builtin(__builtin_memcmp)
#define memcmp(p1, p2, num) __builtin_memcmp(p1, p2, num)
#endif

#if __has_builtin(__builtin_strcmp)
#define strcmp(s1, s2) __builtin_strcmp(s1, s2)
#endif

#if __has_builtin(__builtin_strncmp)
#define strncmp(s1, s2, num) __builtin_strncmp(s1, s2, num)
#endif

#if __has_builtin(__builtin_strncmp)
#define memchr(ptr, val, num) __builtin_memchr(ptr, val, num)
#endif

#if __has_builtin(__builtin_strchr)
#define strchr(str, chr) __builtin_strchr(str, chr)
#endif

#if __has_builtin(__builtin_strcspn)
#define strcspn(s1, s2) __builtin_strcspn(s1, s2)
#endif

#if __has_builtin(__builtin_strpbrk)
#define strpbrk(s1, s2) __builtin_strpbrk(s1, s2)
#endif

#if __has_builtin(__builtin_strchr)
#define strrchr(s1, s2) __builtin_strrchr(s1, s2)
#endif

#if __has_builtin(__builtin_strspn)
#define strspn(s1, s2) __builtin_strspn(s1, s2)
#endif

#if __has_builtin(__builtin_strstr)
#define strstr(s1, s2) __builtin_strstr(s1, s2)
#endif

#if __has_builtin(__builtin_strtok)
#define strtok(s1, s2) __builtin_strtok(s1, s2)
#endif

#if __has_builtin(__builtin_memset)
#define memset(p, v, n) __builtin_memset(p, v, n)
#endif

#if __has_builtin(__builtin_strlen)
#define strlen(s) __builtin_strlen(s)
#endif

#endif /* KMI_STRING_H */
