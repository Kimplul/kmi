KERNEL_LOCAL	!= echo $(ARCH_SOURCE)/kernel/*.[cS]
KERNEL_SOURCES	+= $(KERNEL_LOCAL)
INIT_SOURCES	+= $(ARCH_SOURCE)/init/*.[cS]

# this doesn't work for rv32, but fine for now */
ARCH_CFLAGS	+= -mcmodel=medany

# oof, LLVM's riscv support has become better since I last looked into
# it, but for some reason LTO still causes a crash. Meaning it has to be
# disabled again, though for a different reason than last time
OPTFLAGS	!= [ "$(LLVM)" != "0" ] && echo $(OPTFLAGS:-flto=) || echo $(OPTFLAGS)

run:
	$(ARCH_SOURCE)/conf/mkimage.sh $(ARCH)

include $(ARCH_SOURCE)/asm/source.mk

# dependecy generation
$(ARCH_KERN_BUILD)/kernel/entry.o: $(ARCH_SOURCE)/kernel/gen/asm-offsets.h

# full cleanup
CLEANUP_CMD	:= $(ARCH_SOURCE)/conf/rmimage.sh
