KERNEL_LOCAL	!= echo arch/riscv/kernel/*.[cS] arch/riscv/common/*.[cS]
KERNEL_SOURCES	+= $(KERNEL_LOCAL)

INIT_LOCAL	!= echo arch/riscv/init/*.[cS] arch/riscv/common/*.[cS]
INIT_SOURCES	+= $(INIT_LOCAL)

CLEANUP_CMD	:= ./arch/riscv/conf/rmimage.sh

ARCH_FLAGS	:= -mcmodel=medany

run:
	./arch/riscv/conf/mkimage.sh
