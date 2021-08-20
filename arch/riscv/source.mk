KERNEL_LOCAL	!= echo arch/riscv/*.[cS]
KERNEL_SOURCES	+= $(KERNEL_LOCAL)

INIT_LOCAL	!= echo arch/riscv/init/*.[cS]
INIT_SOURCES	+= $(INIT_LOCAL)
