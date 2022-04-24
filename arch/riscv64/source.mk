KERNEL_LOCAL	!= echo arch/riscv64/kernel/*.[cS]
KERNEL_SOURCES	+= $(KERNEL_LOCAL)
INIT_SOURCES	+= arch/riscv64/init/*.[cS]

CLEANUP_CMD	:= ./arch/riscv64/conf/rmimage.sh

ARCH_CFLAGS	:= -mcmodel=medany
ARCH_LDFLAGS	:=

# llvm compilation doesn't seem to work, either bfd reads the object files wrong
# and outputs incorrect symbols or lld handles relocations in a dumb way. Only
# GCC compiles the kernel correctly at the moment (on RISCV, at least). I'll
# look into this later, but for now I'll just use GCC. The code does _compile_
# with clang, it just won't link.
#LINKFLAGS	:= -fuse-ld=bfd

run:
	./arch/riscv64/conf/mkimage.sh
