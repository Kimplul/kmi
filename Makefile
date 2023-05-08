DO		!= echo -n > deps.mk

# this could be done better
DEBUGFLAGS	!= [ $(RELEASE) ] \
			&& echo "-flto -O2 -DNDEBUG" \
			|| echo "-O0 -DDEBUG"

CFLAGS		= -ffreestanding -nostdlib -static -fno-pie -std=c17 -Wall -Wextra -Wvla -D$(ARCH) -g
DEPFLAGS	= -MT $@ -MMD -MP -MF $@.d
LINTFLAGS	= -fsyntax-only
PREPROCESS	= -E
LDFLAGS		!= [ $(LLVM) ] \
			|| echo -static-libgcc -lgcc

BUILD		= build
ARCH_BUILD	= $(BUILD)/arch/$(ARCH)
ARCH_SOURCE	= arch/$(ARCH)

all: kmi.bin
# bmake requires us to run make depend to genereate deps.mk beforehand,
# so create an empty rule that lets the rest of the script do its job
depend:

# default values, overwrite if/when needed
ARCH		?= riscv64
CROSS_COMPILE	?= $(ARCH)-unknown-elf-

OBJCOPY		!= [ $(LLVM) ] \
			&& echo llvm-objcopy \
			|| echo $(CROSS_COMPILE)objcopy

COMPILER	!= [ $(LLVM) ] \
			&& echo clang --target="$(CROSS_COMPILE)" \
			|| echo $(CROSS_COMPILE)gcc


KERNEL_SOURCES	!= echo common/*.c common/uapi/*.c lib/*.c
CLEANUP		:= build deps.mk kernel.* init.* kmi.bin
CLEANUP_CMD	:=
INIT_SOURCES	:=

include arch/$(ARCH)/source.mk

COMPILE_FLAGS	:= $(CFLAGS) $(ARCH_CFLAGS)
LINK_FLAGS	:= $(LDFLAGS) $(ARCH_LDFLAGS)

INCLUDE_FLAGS	:= -I include -include config.h -include arch/$(ARCH)/config.h

# This makes sure .bss is loaded into the binary
OBJCOPY_FLAGS	?= -Obinary -R .garbage \
		   --set-section-flags .bss=alloc,load,contents

COMPILE		= $(COMPILER) $(DEBUGFLAGS)\
		  $(COMPILE_FLAGS) $(DEPFLAGS) $(INCLUDE_FLAGS)

LINT		= $(COMPILER) $(DEBUGFLAGS)\
		  $(COMPILE_FLAGS) $(LINTFLAGS) $(INCLUDE_FLAGS)

GENELF		= $(COMPILER) $(DEBUGFLAGS)\
		  $(COMPILE_FLAGS) $(INCLUDE_FLAGS)

GENLINK		= $(COMPILER) $(COMPILE_FLAGS) $(PREPROCESS) $(DEPFLAGS) $(INCLUDE_FLAGS)
STRIPLINK	= sed -n '/^[^\#]/p'
KERN_SIZE	= wc -c kernel.bin | awk '{print $$1}'
KERN_INFO	= sed "s/<KERNEL_SIZE>/$$($(KERN_SIZE))/"

KERNEL_LINK	:= arch/$(ARCH)/conf/kernel-link
INIT_LINK	:= arch/$(ARCH)/conf/init-link

KERN_FLAGS	!= [ $(UBSAN) ] && echo -fsanitize=undefined || echo
INIT_FLAGS	:=

KERNEL_OBJECTS	!= ./scripts/gen-deps --kernel --compile "$(KERNEL_SOURCES)"
INIT_OBJECTS	!= ./scripts/gen-deps --init --compile "$(INIT_SOURCES)"
KERNEL_LD	!= ./scripts/gen-deps --kernel --link "$(KERNEL_LINK).S"
INIT_LD		!= ./scripts/gen-deps --init --link "$(INIT_LINK).S"

-include deps.mk

$(INIT_LD): kernel.bin

init.elf: $(INIT_OBJECTS) $(INIT_LD)
	$(GENELF) -T $(INIT_LD) $(INIT_OBJECTS) -o init.elf $(LINK_FLAGS)

kernel.elf: $(KERNEL_OBJECTS) $(KERNEL_LD)
	$(GENELF) -T $(KERNEL_LD) $(KERNEL_OBJECTS) -o kernel.elf $(LINK_FLAGS)

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
