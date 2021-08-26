#ifndef APOS_COMPILER_ATTRIBUTES_H

#define __section(section)	__attribute__((__section__(section)))
#define __fmt(x, y)		__attribute__((format (__printf__, x, y)))
#define __noinline		__attribute__((noinline))
#define __weak			__attribute__((weak))
#define __packed		__attribute__((packed))

#endif /* APOS_COMPILER_ATTRIBUTES_H */
