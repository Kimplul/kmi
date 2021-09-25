#include <apos/sizes.h>
#define PM_KERN		0x83800000
#define PM_STACK_BASE	(PM_KERN + SZ_2M)
#define PM_STACK_TOP	(PM_STACK_BASE + SZ_2M - 2)
#define VM_KERN ((-1) - SZ_1G)

#define MAX_ORDER 3
#define PAGE_SHIFT 12
#define O0_WIDTH 9
#define O1_WIDTH 9
#define O2_WIDTH 9
#define O3_WIDTH 9
