KERNEL_LOCAL	!= echo $(ARCH_SOURCE)/kernel/*.[cS]
KERNEL_SOURCES	+= $(KERNEL_LOCAL)
INIT_SOURCES	+= $(ARCH_SOURCE)/init/*.[cS]

# this doesn't work for rv32, but fine for now */
CLEANUP_CMD	:= $(ARCH_SOURCE)/conf/rmimage.sh

ARCH_CFLAGS	:= -mcmodel=medany
ARCH_LDFLAGS	:=

# llvm compilation doesn't seem to work, either bfd reads the object files wrong
# and outputs incorrect symbols or lld handles relocations in a dumb way. Only
# GCC compiles the kernel correctly at the moment (on RISCV, at least). I'll
# look into this later, but for now I'll just use GCC. The code does _compile_
# with clang, it just won't link.
#LINKFLAGS	:= -fuse-ld=bfd

run:
	$(ARCH_SOURCE)/conf/mkimage.sh

include $(ARCH_SOURCE)/gen/source.mk

# dependecy generation
$(ARCH_BUILD)/kernel/head.o: $(ARCH_SOURCE)/include/gen/asm-offsets.h
