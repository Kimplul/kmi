#ifndef APOS_RISCV_PAGES_H
#define APOS_RISCV_PAGES_H

typedef char pagemask_t;
typedef char lockbit_t;

struct kilopages {
	pagemask_t page[64];
};

struct megapages {
	lockbit_t lockbit[64];
	pagemask_t empty[64];
	pagemask_t full[64];
	struct kilopages kilo[512];
};

/* Sv39 */
struct gigapages {
	lockbit_t lockbit[64];
	pagemask_t empty[64];
	pagemask_t full[64];
	struct megapages mega[512];
};

struct terapages {
	lockbit_t lockbit[64];
	pagemask_t empty[64];
	pagemask_t full[64];
	struct gigapages giga[512];
};

struct mm_pagearr_info {
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
