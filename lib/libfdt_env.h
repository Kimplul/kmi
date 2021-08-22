#ifndef LIBFDT_ENV_H
/* take over libfdt */
#define LIBFDT_ENV_H

#include <apos/types.h>
#include <apos/string.h>

typedef int16_t fdt16_t;
typedef int32_t fdt32_t;
typedef int64_t fdt64_t;

#define fdt32_to_cpu(x) be32_to_cpu(x)
#define cpu_to_fdt32(x) cpu_to_be32(x)
#define fdt64_to_cpu(x) be64_to_cpu(x)
#define cpu_to_fdt64(x) cpu_to_be64(x)

#endif /* LIBFDT_ENV_H */
