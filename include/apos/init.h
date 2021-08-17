#ifndef APOS_INIT_H
#define APOS_INIT_H

#include <apos/compiler_attributes.h>

#define __init __section(".init.text")
#define __initdata __section(".init.data")

#endif /* APOS_INIT_H */
