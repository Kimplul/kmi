#ifndef APOS_CSR_H
#define APOS_CSR_H

#include <apos/utils.h>

#define SATP_MODE_Sv32	0x80000000
#define SATP_MODE_Sv39	0x8000000000000000
#define SATP_MODE_Sv48	0x9000000000000000


/* supervisor CSR registers */
#define CSR_SSTATUS	0x100
#define CSR_SIE		0x104
#define CSR_STVEC	0x105
#define CSR_SCOUNTEREN	0x106

#define CSR_SENVCFG	0x10A

#define CSR_SSCRATCH	0x140
#define CSR_SEPC	0x141
#define CSR_SCAUSE	0x142
#define CSR_STVAL	0x143
#define CSR_SIP		0x144

#define CSR_SATP	0x180

#define CSR_SCONTEXT	0x5A8

/* Exception causes */
#define EXC_INST_MISALIGNED	0
#define EXC_INST_ACCESS		1
#define EXC_BREAKPOINT		3
#define EXC_LOAD_ACCESS		5
#define EXC_STORE_ACCESS	7
#define EXC_SYSCALL		8
#define EXC_INST_PAGE_FAULT	12
#define EXC_LOAD_PAGE_FAULT	13
#define EXC_STORE_PAGE_FAULT	15

/* directly lifted from Linux:/arch/riscv/include/asm/asm.h:9-13 */
#ifdef __ASSEMBLY__
#define __ASM_STR(x) x
#else
#define __ASM_STR(x) #x
#endif

#define csr_read(csr, res)\
	__asm__ ("csrr %0, " __ASM_STR(csr) : "=r" (res) : : "memory")

#define csr_write(csr, val)\
	__asm__ ("csrw " __ASM_STR(csr) ", %0" : : "rK" (val) : "memory")

#endif /* APOS_CSR_H */
