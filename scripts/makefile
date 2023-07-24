# this could be done better
RELEASE		?= 0
OPTFLAGS	!= [ "$(RELEASE)" != "0" ] \
			&& echo "-O3 -flto" \
			|| echo "-O0"

DEBUG		?= 1
DEBUGFLAGS	!= [ "$(DEBUG)" != "0" ] \
			&& echo "-DDEBUG=1" \
			|| echo "-DNDEBUG=1"

GENERIC_UBOOT	?= 0
UBOOTFLAGS	!= [ "$(GENERIC_UBOOT)" != "0" ] \
			&& echo "-DGENERIC_UBOOT=1" \
			|| echo

ASSERT		?= 1
ASSERTFLAGS	!= [ "$(ASSERT)" != "0" ] \
			&& echo "-DASSERT=1" \
			|| echo

DEPFLAGS	= -MT $@ -MMD -MP -MF $@.d
LINTFLAGS	= -fsyntax-only
PREPROCESS	= -E

LLVM		?= 0
LDFLAGS		!= [ "$(LLVM)" = "0" ] \
			&& echo -static-libgcc -lgcc \
			|| echo

BUILD		= build
ARCH_KERN_BUILD	= $(BUILD)/kernel/arch/$(ARCH)
ARCH_INIT_BUILD = $(BUILD)/init/arch/$(ARCH)
ARCH_SOURCE	= arch/$(ARCH)

all: kmi.bin

# default values, overwrite if/when needed
ARCH		?= riscv64
CROSS_COMPILE	?= $(ARCH)-unknown-elf-

OBJCOPY		!= [ "$(LLVM)" != "0" ] \
			&& echo llvm-objcopy \
			|| echo $(CROSS_COMPILE)objcopy

COMPILER	!= [ "$(LLVM)" != "0" ] \
			&& echo clang --target="$(CROSS_COMPILE)" \
			|| echo $(CROSS_COMPILE)gcc


OBFLAGS		= -ffreestanding -nostdlib -static -fno-pie -std=c17 -g
WARNFLAGS	= -Wall -Wextra -Wvla
ARCH_CFLAGS	= -D$(ARCH)

include arch/$(ARCH)/source.mk

COMPILE_FLAGS	= $(CFLAGS) $(WARNFLAGS) $(OPTFLAGS) $(OBFLAGS) $(ASSERTFLAGS) \
		   $(DEBUGFLAGS) $(UBOOTFLAGS) $(ARCH_CFLAGS)

LINK_FLAGS	= $(LDFLAGS) $(ARCH_LDFLAGS)

INCLUDE_FLAGS	= -I include -include config.h -include arch/$(ARCH)/config.h

# This makes sure .bss is loaded into the binary
OBJCOPY_FLAGS	?= -Obinary -R .garbage \
		   --set-section-flags .bss=alloc,load,contents

COMPILE		= $(COMPILER) \
		  $(COMPILE_FLAGS) $(DEPFLAGS) $(INCLUDE_FLAGS)

LINT		= $(COMPILER) \
		  $(COMPILE_FLAGS) $(LINTFLAGS) $(INCLUDE_FLAGS)

GENELF		= $(COMPILER) \
		  $(COMPILE_FLAGS) $(INCLUDE_FLAGS)

GENLINK		= $(COMPILER) $(COMPILE_FLAGS) $(PREPROCESS) $(DEPFLAGS) $(INCLUDE_FLAGS)
STRIPLINK	= sed -n '/^[^\#]/p'
KERN_SIZE	= wc -c kernel.bin | awk '{print $$1}'
KERN_INFO	= sed "s/<KERNEL_SIZE>/$$($(KERN_SIZE))/"

KERNEL_LINK	= arch/$(ARCH)/conf/kernel-link.S
INIT_LINK	= arch/$(ARCH)/conf/init-link.S

KERNEL_LD	= build/kernel-link.ld
INIT_LD		= build/init-link.ld

UBSAN		?= 0
KERN_FLAGS	!= [ "$(UBSAN)" != "0" ] \
			&& echo -fsanitize=undefined \
			|| echo

INIT_FLAGS	:= -fpic

COMPILE_KERNEL	= $(COMPILE) $(KERN_FLAGS)
COMPILE_INIT	= $(COMPILE) $(INIT_FLAGS)

-include deps.mk

-include $(KERNEL_LD).d
$(INIT_LD): kernel.bin
$(INIT_LD): $(INIT_LINK)
	$(GENLINK) $(INIT_LINK) | $(STRIPLINK) | $(KERN_INFO) > $(INIT_LD)

-include $(KERNEL_LD).d
$(KERNEL_LD): $(KERNEL_LINK)
	$(GENLINK) $(KERNEL_LINK) | $(STRIPLINK) > $(KERNEL_LD)

init.elf: $(INIT_OBJS) $(INIT_LD)
	$(GENELF) $(INIT_FLAGS) -T $(INIT_LD) $(INIT_OBJS) -o init.elf $(LINK_FLAGS)

kernel.elf: $(KERNEL_OBJS) $(KERNEL_LD)
	$(GENELF) $(KERNEL_FLAGS) -T $(KERNEL_LD) $(KERNEL_OBJS) -o kernel.elf $(LINK_FLAGS)

init.bin: init.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) init.elf init.bin

kernel.bin: kernel.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) kernel.elf kernel.bin

kmi.bin: init.bin kernel.bin
	cat init.bin kernel.bin > kmi.bin

# might lint some common things twice
.PHONY:
lint: $(INIT_LINTS) $(KERNEL_LINTS)
