DO		!= echo -n > deps.mk

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
			&& echo "-DGENERIC_UBOOT=1"

ASSERT		?= 1
ASSERTFLAGS	!= [ "$(ASSERT)" != "0" ] \
			&& echo "-DASSERT=1"

DEPFLAGS	= -MT $@ -MMD -MP -MF $@.d
LINTFLAGS	= -fsyntax-only
PREPROCESS	= -E

LLVM		?= 0
LDFLAGS		!= [ "$(LLVM)" = "0" ] \
			&& echo -static-libgcc -lgcc

BUILD		= build
ARCH_KERN_BUILD	= $(BUILD)/kernel/arch/$(ARCH)
ARCH_INIT_BUILD = $(BUILD)/init/arch/$(ARCH)
ARCH_SOURCE	= arch/$(ARCH)

all: kmi.bin
# bmake requires us to run make depend to genereate deps.mk beforehand,
# so create an empty rule that lets the rest of the script do its job
depend:

# default values, overwrite if/when needed
ARCH		?= riscv64
CROSS_COMPILE	?= $(ARCH)-unknown-elf-

OBJCOPY		!= [ "$(LLVM)" != "0" ] \
			&& echo llvm-objcopy \
			|| echo $(CROSS_COMPILE)objcopy

COMPILER	!= [ "$(LLVM)" != "0" ] \
			&& echo clang --target="$(CROSS_COMPILE)" \
			|| echo $(CROSS_COMPILE)gcc


KERNEL_SOURCES	!= echo common/*.c common/uapi/*.c lib/*.c
# might consider renaming common, currently it refers to stuff common
# to all arches but clearly there are bits that are common to init and kernel
INIT_SOURCES	!= echo lib/fdt*.c common/fdt.c common/string.c
CLEANUP		:= build deps.mk kernel.* init.* kmi.bin
CLEANUP_CMD	:=

OBFLAGS		:= -ffreestanding -nostdlib -static -fno-pie -std=c17 -g
WARNFLAGS	:= -Wall -Wextra -Wvla
ARCH_CFLAGS	:= -D$(ARCH)

include arch/$(ARCH)/source.mk

COMPILE_FLAGS	:= $(CFLAGS) $(WARNFLAGS) $(OPTFLAGS) $(OBFLAGS) $(ASSERTFLAGS) \
		   $(DEBUGFLAGS) $(UBOOTFLAGS) $(ARCH_CFLAGS)

LINK_FLAGS	:= $(LDFLAGS) $(ARCH_LDFLAGS)

INCLUDE_FLAGS	:= -I include -include config.h -include arch/$(ARCH)/config.h

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

KERNEL_LINK	:= arch/$(ARCH)/conf/kernel-link
INIT_LINK	:= arch/$(ARCH)/conf/init-link

UBSAN		?= 0
KERN_FLAGS	!= [ "$(UBSAN)" != "0" ] \
			&& echo -fsanitize=undefined

INIT_FLAGS	:= -fpic

KERNEL_OBJECTS	!= ./scripts/gen-deps --kernel --compile "$(KERNEL_SOURCES)"
INIT_OBJECTS	!= ./scripts/gen-deps --init --compile "$(INIT_SOURCES)"
KERNEL_LD	!= ./scripts/gen-deps --kernel --link "$(KERNEL_LINK).S"
INIT_LD		!= ./scripts/gen-deps --init --link "$(INIT_LINK).S"

-include deps.mk

$(INIT_LD): kernel.bin

init.elf: $(INIT_OBJECTS) $(INIT_LD)
	$(GENELF) $(INIT_FLAGS) -T $(INIT_LD) $(INIT_OBJECTS) -o init.elf $(LINK_FLAGS)

kernel.elf: $(KERNEL_OBJECTS) $(KERNEL_LD)
	$(GENELF) $(KERNEL_FLAGS) -T $(KERNEL_LD) $(KERNEL_OBJECTS) -o kernel.elf $(LINK_FLAGS)

init.bin: init.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) init.elf init.bin

kernel.bin: kernel.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) kernel.elf kernel.bin

kmi.bin: init.bin kernel.bin
	cat init.bin kernel.bin > kmi.bin

.PHONY:
lint: $(INIT_OBJECTS:.o=.o.l) $(KERNEL_OBJECTS:.o=.o.l)

.PHONY: format
format:
	find arch lib common include -iname '*.[ch]' |\
		xargs -n 10 -P 0 uncrustify -c uncrustify.conf --no-backup -F -

.PHONY: license
license:
	find arch lib common include -iname '*.[ch]' |\
		xargs -n 10 -P 0 ./scripts/license

.PHONY: docs
docs:
	find arch lib common include -iname '*.[ch]' -not -path */gen/* |\
		xargs ./scripts/warn-undocumented
	doxygen docs/doxygen.conf

# bmake didn't seem to have the -f flag on by default
RM	?= rm -f

.PHONY: clean
clean:
	$(RM) -r $(CLEANUP)

.PHONY: clean_run
clean_run:
	$(CLEANUP_CMD)

.PHONY: clean_docs
clean_docs:
	$(RM) -r docs/output

.PHONY: clean_all
clean_all: clean clean_run clean_docs
