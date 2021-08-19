KERNEL_LOCAL	!= ./scripts/prepend arch/riscv/ main.c
KERNEL_SOURCES	+= $(KERNEL_LOCAL)

INIT_LOCAL	!= ./scripts/prepend arch/riscv/init/ init.c head.S
INIT_SOURCES	+= $(INIT_LOCAL)

INCLUDE_DIRS	+= arch/riscv/include
KERNEL_LINK	:= arch/riscv/kernel-link.S
INIT_LINK	:= arch/riscv/init-link.S
