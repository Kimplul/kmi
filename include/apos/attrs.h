#ifndef APOS_COMPILER_ATTRIBUTES_H
#define APOS_COMPILER_ATTRIBUTES_H

/**
 * @file attrs.h
 * Attribute shorthands.
 */

/**
 * Ask the preprocessor if specified attribute is available.
 * Currently unused, but I really should use it.
 *
 * @param x Attribute whose existence should be checked.
 * @return Non-zero when available, \c 0 when not available.
 * @todo Figure out which attributes are necessary and which are good to have
 */
#if !defined(__has_attribute)
#define __has_attribute(x) 0
#endif

/**
 * Place object into section.
 *
 * @param section Section name to place the object in.
 */
#define __section(section) __attribute__((__section__(section)))

/**
 * Inform the compiler that the function uses printf formatting.
 *
 * @param x Index of the format string.
 * @param y Index of variadic arguments.
 */
#define __fmt(x, y) __attribute__((format(__printf__, x, y)))

/**
 * Align object.
 *
 * @param a Value to which object should be aligned.
 */
#define __aligned(a) __attribute__((aligned(a)))

/** Don't inline function. */
#define __noinline __attribute__((noinline))

/** Function should not return. */
#define __noreturn __attribute__((noreturn))

/** Pack structure. */
#define __packed __attribute__((packed))

/** Weak linkage. */
#define __weak __attribute__((weak))

/** Main entry point of kernel proper. */
#define __main __section(".kernel.start") __noinline

/** Entry point of kernel loader. */
#define __init __section(".init.start") __noinline

#endif /* APOS_COMPILER_ATTRIBUTES_H */
