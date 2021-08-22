KERNEL_LOCAL	!= echo arch/riscv/*.[cS]
KERNEL_SOURCES	+= $(KERNEL_LOCAL)

INIT_LOCAL	!= echo arch/riscv/init/*.[cS]
INIT_FDT	!= echo lib/fdt*.c
INIT_SOURCES	+= $(INIT_LOCAL) $(INIT_FDT)

CLEANUP_CMD	:= ./arch/riscv/conf/rmimage.sh

USE_FDT		:= y

run:
	./arch/riscv/conf/mkimage.sh
