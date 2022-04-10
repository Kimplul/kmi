#ifndef APOS_COMPILER_ATTRIBUTES_H

#define __section(section) __attribute__((__section__(section)))
#define __fmt(x, y)        __attribute__((format(__printf__, x, y)))
#define __aligned(a)       __attribute__((aligned(a)))
#define __noinline         __attribute__((noinline))
#define __noreturn         __attribute__((noreturn))
#define __packed           __attribute__((packed))
#define __weak             __attribute__((weak))

#define __main             __section(".kernel.start") __noinline
#define __init             __section(".init.start") __noinline

#endif /* APOS_COMPILER_ATTRIBUTES_H */
