#ifndef APOS_COMPILER_ATTRIBUTES_H

#define __section(section)	__attribute__((__section__(section)))
#define __noinline		__attribute__((noinline))
#define __weak			__attribute((weak))

#endif /* APOS_COMPILER_ATTRIBUTES_H */
