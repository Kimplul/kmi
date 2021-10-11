#include <apos/sizes.h>

/* --- START ARCH USER CONFIG VALUES --- */
/* physical address to which the kernel will be loaded */
#define PM_KERN		0x83800000
/* --- END ARCH USER CONFIG VALUES --- */

/* don't touch >:( */

#define PM_STACK_BASE	(PM_KERN + SZ_2M)
#define PM_STACK_TOP	(PM_STACK_BASE + SZ_2M - 2)
#define VM_KERN		(-SZ_1G)
