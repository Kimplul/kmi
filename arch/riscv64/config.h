#include <apos/sizes.h>

/* --- START ARCH USER CONFIG VALUES --- */
/* physical address to which the kernel will be loaded */
#define RAM_BASE        0x80000000
#define PM_KERN_BASE    (RAM_BASE + SZ_512K)
#define PM_KERN_SIZE    (SZ_256K)
#define PM_KERN_TOP     (PM_KERN_BASE + PM_KERN_SIZE)
/* --- END ARCH USER CONFIG VALUES --- */

/* don't touch >:( */

#define PM_STACK_BASE   (PM_KERN_BASE + SZ_256K)
#define PM_STACK_SIZE   (SZ_256K)
#define PM_STACK_TOP    (PM_STACK_BASE + PM_STACK_SIZE)

#if __riscv_xlen == 64
/* 64bit */
#define VM_DMAP         (0xffffffc000000000) /* testing for now */
#define VM_KERN         (VM_DMAP + SZ_256K)

#define IO_PAGE         511UL
#define KSTART_PAGE     256UL
#define CSTACK_PAGE     255UL

/* assume Sv39, probably wouldn't be too difficult to use runtime parameters
 * instead. First 4K is reserved for NULL, but I suppose it could be mapped
 * later if *absolutely* necessary. */
#define UVMEM_START     (SZ_4K)
#define UVMEM_END       (SZ_256G - SZ_1G)

#define PROC_STACK_TOP  (SZ_256G)
#define PROC_STACK_BASE (SZ_256G - SZ_1G)
#else
/* 32bit */
/* TODO: figure this stuff out */
#define VM_DMAP         (0x000000000)
#define VM_KERN         (VM_DMAP + SZ_256K)
#define ROOT_PTE        (0UL)
#define ROOT_REGION     (SZ_4K)

#define IO_PAGE         1023UL
#define KSTART_PAGE     512UL
#define CSTACK_PAGE     511UL

/* assume Sv39, probably wouldn't be too difficult to use runtime parameters
 * instead. First 4K is reserved for NULL, but I suppose it could be mapped
 * later if *absolutely* necessary. */
#define UVMEM_START     (SZ_4K)
#define UVMEM_END       (SZ_4G - SZ_8M)

#define PROC_STACK_TOP  (SZ_4G)
#define PROC_STACK_BASE (SZ_4G - SZ_8M)
#endif
