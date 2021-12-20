COMMON_LOCAL	!= echo arch/riscv/common/*.[cS]
KERNEL_LOCAL	!= echo arch/riscv/kernel/*.[cS]
KERNEL_SOURCES	+= $(KERNEL_LOCAL) $(COMMON_LOCAL)
INIT_LOCAL	!= echo arch/riscv/init/*.[cS]
INIT_SOURCES	+= $(INIT_LOCAL) $(COMMON_LOCAL)

CLEANUP_CMD	:= ./arch/riscv/conf/rmimage.sh

ARCH_FLAGS	:= -mcmodel=medany

run:
	./arch/riscv/conf/mkimage.sh
