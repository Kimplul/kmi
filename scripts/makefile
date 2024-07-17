# this could be done better
CFLAGS		?=

OPTFLAGS	!= [ "$(RELEASE)" -a "$(RELEASE)" != "0" ] \
			&& echo "-O2 -flto=auto" \
			|| echo "-O0"

DEBUGFLAGS	!= [ "$(DEBUG)" != "0" ] \
			&& echo "-DDEBUG=1" \
			|| echo "-DNDEBUG=1"

UBOOTFLAGS	!= [ "$(GENERIC_UBOOT)" -a "$(GENERIC_UBOOT)" != "0" ] \
			&& echo "-DGENERIC_UBOOT=1" \
			|| echo

ASSERTFLAGS	!= [ "$(ASSERT)" != "0" ] \
			&& echo "-DASSERT=1" \
			|| echo

USE_LLVM	!= [ "$(LLVM)" -a "$(LLVM)" != "0" ] \
			&& echo "1"

DEPFLAGS	= -MT $@ -MMD -MP -MF $@.d
LINTFLAGS	= -fsyntax-only
PREPROCESS	= -E

BUILD		= build
ARCH_KERN_BUILD	= $(BUILD)/kernel/arch/$(ARCH)
ARCH_SOURCE	= arch/$(ARCH)

all: kmi.bin

# default values, overwrite if/when needed
ARCH		?= riscv64
CROSS_COMPILE	?= $(ARCH)-unknown-elf-

OBJCOPY		!= [ "$(USE_LLVM)" ] \
			&& echo llvm-objcopy \
			|| echo $(CROSS_COMPILE)objcopy

COMPILER	!= [ "$(USE_LLVM)" ] \
			&& echo clang --target="$(CROSS_COMPILE)" \
			|| echo $(CROSS_COMPILE)gcc


OBFLAGS		= -ffreestanding -nostdlib -static -fpic -fno-pie -std=c17 -g
WARNFLAGS	= -Wall -Wextra -Wvla
ARCH_CFLAGS	= -D$(ARCH)

include arch/$(ARCH)/source.mk

COMPILE_FLAGS	= $(CFLAGS) $(WARNFLAGS) $(OPTFLAGS) $(OBFLAGS) $(ASSERTFLAGS) \
		   $(DEBUGFLAGS) $(UBOOTFLAGS) $(ARCH_CFLAGS)

LINK_FLAGS	= $(LDFLAGS) $(ARCH_LDFLAGS)

INCLUDE_FLAGS	= -I include -include config.h -include arch/$(ARCH)/config.h

# This makes sure .bss is loaded into the binary
OBJCOPY_FLAGS	?= -Obinary -R .garbage

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

KERNEL_LD	= build/kernel-link.ld

UBSAN		?= 0
KERN_FLAGS	!= [ "$(UBSAN)" != "0" ] \
			&& echo -fsanitize=undefined \
			|| echo

COMPILE_KERNEL	= $(COMPILE) $(KERN_FLAGS)
COMPILE_INIT	= $(COMPILE) $(INIT_FLAGS)

-include deps.mk

-include $(KERNEL_LD).d
$(KERNEL_LD): $(KERNEL_LINK)
	$(GENLINK) $(KERNEL_LINK) | $(STRIPLINK) > $(KERNEL_LD)

kmi.elf: $(KERNEL_OBJS) $(KERNEL_LD)
	$(GENELF) $(KERNEL_FLAGS) -T $(KERNEL_LD) $(KERNEL_OBJS) -o kmi.elf $(LINK_FLAGS)

kmi.bin: kmi.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) kmi.elf kmi.bin

# might lint some common things twice
.PHONY:
lint: $(KERNEL_LINTS)
