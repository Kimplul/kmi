KERNEL_LOCAL	!= echo arch/riscv/kernel/*.[cS]
KERNEL_SOURCES	+= $(KERNEL_LOCAL)

CLEANUP_CMD	:= ./arch/riscv/conf/rmimage.sh

ARCH_FLAGS	:= -mcmodel=medany

run:
	./arch/riscv/conf/mkimage.sh
