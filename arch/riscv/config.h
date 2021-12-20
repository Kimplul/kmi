#include <apos/sizes.h>

/* --- START ARCH USER CONFIG VALUES --- */
/* physical address to which the kernel will be loaded */
#define RAM_BASE	0x80000000
#define PM_KERN		(RAM_BASE + SZ_512K)
/* --- END ARCH USER CONFIG VALUES --- */

/* don't touch >:( */

#define PM_STACK_BASE	(PM_KERN + SZ_2M)
#define PM_STACK_TOP	(PM_STACK_BASE + SZ_2M - 2)
#define VM_DMAP		(-SZ_256G) /* testing for now */
#define VM_KERN		(VM_DMAP + SZ_256K)
#define TMP_PTE		(-SZ_2G)
#define ROOT_PTE	(0)
#define ROOT_REGION	(SZ_4K)
