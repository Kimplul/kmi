KERNEL_LOCAL	!= echo $(ARCH_SOURCE)/kernel/*.[cS]
KERNEL_SOURCES	:= $(KERNEL_SOURCES) $(KERNEL_LOCAL)

# this doesn't work for rv32, but fine for now */
ARCH_CFLAGS	:= $(ARCH_CFLAGS) -mcmodel=medany

include $(ARCH_SOURCE)/asm/source.mk

# dependecy generation
$(ARCH_KERN_BUILD)/kernel/entry.o: $(ARCH_SOURCE)/kernel/gen/asm-offsets.h
