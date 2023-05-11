KERNEL_LOCAL	!= echo $(ARCH_SOURCE)/kernel/*.[cS]
KERNEL_SOURCES	+= $(KERNEL_LOCAL)
INIT_SOURCES	+= $(ARCH_SOURCE)/init/*.[cS]

# this doesn't work for rv32, but fine for now */
ARCH_CFLAGS	:= -mcmodel=medany

run:
	$(ARCH_SOURCE)/conf/mkimage.sh $(ARCH)

include $(ARCH_SOURCE)/asm/source.mk

# dependecy generation
$(ARCH_KERN_BUILD)/kernel/entry.o: $(ARCH_SOURCE)/kernel/gen/asm-offsets.h

# full cleanup
CLEANUP_CMD	:= $(ARCH_SOURCE)/conf/rmimage.sh
