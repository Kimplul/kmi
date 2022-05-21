#ifndef APOS_DEBUG_H
#define APOS_DEBUG_H

#include <apos/attrs.h>
#include <apos/pmem.h>
#include <arch/vmem.h>

#if _LP64
/* approximate, but probably good enough */
#define __PRI64_PREFIX  "l"
#define __PRIPTR_PREFIX "l"
#else
#define __PRI64_PREFIX "ll"
#define __PRIPTR_PREFIX
#endif

/* Decimal notation.  */
#define PRId8       "d"
#define PRId16      "d"
#define PRId32      "d"
#define PRId64      __PRI64_PREFIX "d"

#define PRIdLEAST8  "d"
#define PRIdLEAST16 "d"
#define PRIdLEAST32 "d"
#define PRIdLEAST64 __PRI64_PREFIX "d"

#define PRIdFAST8   "d"
#define PRIdFAST16  __PRIPTR_PREFIX "d"
#define PRIdFAST32  __PRIPTR_PREFIX "d"
#define PRIdFAST64  __PRI64_PREFIX "d"

#define PRIi8       "i"
#define PRIi16      "i"
#define PRIi32      "i"
#define PRIi64      __PRI64_PREFIX "i"

#define PRIiLEAST8  "i"
#define PRIiLEAST16 "i"
#define PRIiLEAST32 "i"
#define PRIiLEAST64 __PRI64_PREFIX "i"

#define PRIiFAST8   "i"
#define PRIiFAST16  __PRIPTR_PREFIX "i"
#define PRIiFAST32  __PRIPTR_PREFIX "i"
#define PRIiFAST64  __PRI64_PREFIX "i"

/* Octal notation.  */
#define PRIo8       "o"
#define PRIo16      "o"
#define PRIo32      "o"
#define PRIo64      __PRI64_PREFIX "o"

#define PRIoLEAST8  "o"
#define PRIoLEAST16 "o"
#define PRIoLEAST32 "o"
#define PRIoLEAST64 __PRI64_PREFIX "o"

#define PRIoFAST8   "o"
#define PRIoFAST16  __PRIPTR_PREFIX "o"
#define PRIoFAST32  __PRIPTR_PREFIX "o"
#define PRIoFAST64  __PRI64_PREFIX "o"

/* Unsigned integers.  */
#define PRIu8       "u"
#define PRIu16      "u"
#define PRIu32      "u"
#define PRIu64      __PRI64_PREFIX "u"

#define PRIuLEAST8  "u"
#define PRIuLEAST16 "u"
#define PRIuLEAST32 "u"
#define PRIuLEAST64 __PRI64_PREFIX "u"

#define PRIuFAST8   "u"
#define PRIuFAST16  __PRIPTR_PREFIX "u"
#define PRIuFAST32  __PRIPTR_PREFIX "u"
#define PRIuFAST64  __PRI64_PREFIX "u"

/* lowercase hexadecimal notation.  */
#define PRIx8       "x"
#define PRIx16      "x"
#define PRIx32      "x"
#define PRIx64      __PRI64_PREFIX "x"

#define PRIxLEAST8  "x"
#define PRIxLEAST16 "x"
#define PRIxLEAST32 "x"
#define PRIxLEAST64 __PRI64_PREFIX "x"

#define PRIxFAST8   "x"
#define PRIxFAST16  __PRIPTR_PREFIX "x"
#define PRIxFAST32  __PRIPTR_PREFIX "x"
#define PRIxFAST64  __PRI64_PREFIX "x"

/* Binary notation  */
#define PRIX8       "X"
#define PRIX16      "X"
#define PRIX32      "X"
#define PRIX64      __PRI64_PREFIX "X"

#define PRIXLEAST8  "X"
#define PRIXLEAST16 "X"
#define PRIXLEAST32 "X"
#define PRIXLEAST64 __PRI64_PREFIX "X"

#define PRIXFAST8   "X"
#define PRIXFAST16  __PRIPTR_PREFIX "X"
#define PRIXFAST32  __PRIPTR_PREFIX "X"
#define PRIXFAST64  __PRI64_PREFIX "X"

/* Macros for printing `intmax_t' and `uintmax_t'.  */
#define PRIdMAX     __PRI64_PREFIX "d"
#define PRIiMAX     __PRI64_PREFIX "i"
#define PRIoMAX     __PRI64_PREFIX "o"
#define PRIuMAX     __PRI64_PREFIX "u"
#define PRIxMAX     __PRI64_PREFIX "x"
#define PRIXMAX     __PRI64_PREFIX "X"

/* Macros for printing `intptr_t' and `uintptr_t'.  */
#define PRIdPTR     __PRIPTR_PREFIX "d"
#define PRIiPTR     __PRIPTR_PREFIX "i"
#define PRIoPTR     __PRIPTR_PREFIX "o"
#define PRIuPTR     __PRIPTR_PREFIX "u"
#define PRIxPTR     __PRIPTR_PREFIX "x"
#define PRIXPTR     __PRIPTR_PREFIX "X"

#if defined(DEBUG)
enum serial_dev {
	/* only the NS16550A and compatible at the moment */
	NS16550A,
};

void __fmt(1, 2) dbg(const char *fmt, ...);

void init_dbg(const void *fdt);

void setup_dmap_dbg();
void setup_io_dbg(struct vm_branch *b);
void setup_dbg(pm_t pt, enum serial_dev dev);

struct dbg_info dbg_from_fdt(const void *fdt);

#define COMMON_FORMAT  "[%s] %s:%d\n\t"
#define COMMON_ARGS(s) s, __FILE__, __LINE__
#define bug(fmt, ...)  dbg(COMMON_FORMAT fmt, COMMON_ARGS("BUG"), __VA_ARGS__)
#define warn(fmt, ...) dbg(COMMON_FORMAT fmt, COMMON_ARGS("WARN"), __VA_ARGS__)
#define info(fmt, ...) dbg(COMMON_FORMAT fmt, COMMON_ARGS("INFO"), __VA_ARGS__)
#define error(fmt, ...)                                                        \
	dbg(COMMON_FORMAT fmt, COMMON_ARGS("ERROR"), __VA_ARGS__)

#else

#define dbg(...)
#define dbg_init(...)
#define dbg_from_fdt(...)

#define init_dbg(...)
#define setup_dmap_dbg(...)
#define setup_io_dbg(...)

#define bug(...)
#define warn(...)
#define info(...)
#define error(...)

#endif /* DEBUG */

#endif /* APOS_DEBUG_H */
