#ifndef APOS_CSR_H
#define APOS_CSR_H

#define SATP_MODE_39 0x8000000000000000
#define SATP_MODE_48 0x9000000000000000

#define CSR_SATP 0x180

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
