#ifndef APOS_RISCV_PAGES_H
#define APOS_RISCV_PAGES_H

#include <apos/types.h>

typedef uint8_t pagemask_t;
typedef uint8_t lockbit_t;
/* assume riscv64 for now */
typedef uint64_t pm_t;

struct mm_kpages {
	pagemask_t page[64];
};

struct mm_mpages {
	lockbit_t lockbit[64];
	pagemask_t empty[64];
	pagemask_t full[64];
	struct mm_kpages kilo[512];
};

/* Sv39 */
struct mm_gpages {
	lockbit_t lockbit[64];
	pagemask_t empty[64];
	pagemask_t full[64];
	struct mm_mpages mega[512];
};

struct mm_tpages {
	lockbit_t lockbit[64];
	pagemask_t empty[64];
	pagemask_t full[64];
	struct mm_gpages giga[512];
};

struct mm_ptinfo {
	short tera_num;
	struct terapages *tera_top;

	short giga_num;
	struct gigapages *giga_top;

	short mega_num;
	struct megapages *mega_top;

	short kilo_num;
	struct kilopages *kilo_top;
};

#endif /* APOS_RISCV_PAGES_H */
