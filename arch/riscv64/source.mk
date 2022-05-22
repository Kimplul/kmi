KERNEL_LOCAL	!= echo $(ARCH_SOURCE)/kernel/*.[cS]
KERNEL_SOURCES	+= $(KERNEL_LOCAL)
INIT_SOURCES	+= $(ARCH_SOURCE)/init/*.[cS]

# this doesn't work for rv32, but fine for now */
ARCH_CFLAGS	:= -mcmodel=medany
ARCH_LDFLAGS	:= -fuse-ld=bfd

# slightly hacky but bfd is currently the only compiler that correctly handles
# riscv relocations, but it doesn't recognise LLVM lto files, so if we're
# doing an LLVM compilation, remove lto support
DEBUGFLAGS	!= [ $(LLVM) ] && echo $(DEBUGFLAGS:-flto=) || echo $(DEBUGFLAGS)

# llvm-objcopy is buggy, use gnu tools instead
# christ, not much LLVM tooling available for RISCV :D
OBJCOPY		:= $(CROSS_COMPILE)-objcopy

run:
	$(ARCH_SOURCE)/conf/mkimage.sh

include $(ARCH_SOURCE)/gen/source.mk

# dependecy generation
$(ARCH_BUILD)/kernel/entry.o: $(ARCH_SOURCE)/include/gen/asm-offsets.h

# full cleanup
CLEANUP_CMD	:= $(ARCH_SOURCE)/conf/rmimage.sh
