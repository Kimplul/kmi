KERNEL_LOCAL	!= echo arch/riscv/kernel/*.[cS]
KERNEL_SOURCES	+= $(KERNEL_LOCAL)
INIT_SOURCES	+= arch/riscv/init/init.c

CLEANUP_CMD	:= ./arch/riscv/conf/rmimage.sh

ARCH_FLAGS	:= -mcmodel=medany

run:
	./arch/riscv/conf/mkimage.sh
