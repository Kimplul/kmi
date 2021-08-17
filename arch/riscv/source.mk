LOCAL_KERNEL_SOURCES := main.c init/head.S init/init.c
KERNEL_SOURCES += $(addprefix arch/riscv/,$(LOCAL_KERNEL_SOURCES))

INCLUDE_DIRS += arch/riscv/include
LINKER_SOURCE := arch/riscv/riscv-link.S
