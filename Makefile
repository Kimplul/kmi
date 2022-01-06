DO		!= echo > deps.mk

# this could be done better
DEBUGFLAGS	!= [ $(RELEASE) ] \
			&& echo "-flto -O2" \
			|| echo "-O0 -ggdb3 -DDEBUG"

CFLAGS		= -ffreestanding -nostdlib -std=c17 -Wall -Wextra
DEPFLAGS	= -MT $@ -MMD -MP -MF $@.d
LINTFLAGS	= -fsyntax-only
PREPROCESS	= -E
LINKFLAGS	=

all: apos.bin

# default values, overwrite if/when needed
ARCH		?= riscv64
CROSS_COMPILE	?= $(ARCH)-unknown-elf

OBJCOPY		!= [ $(LLVM) ] \
			&& echo llvm-objcopy \
		   	|| echo $(CROSS_COMPILE)-objcopy

COMPILER	!= [ $(LLVM) ] \
			&& echo clang --target="$(CROSS_COMPILE)" \
			|| echo $(CROSS_COMPILE)-gcc


KERNEL_SOURCES	!= echo common/*.c common/uapi/*.c lib/*.c
CLEANUP		:= build deps.mk kernel.* init.* apos.bin
CLEANUP_CMD	:=
INIT_SOURCES	:=

include arch/$(ARCH)/source.mk

INCLUDE_FLAGS	:= -I include -I arch/$(ARCH)/include\
	-include config.h -include arch/$(ARCH)/config.h

# This makes sure .bss is loaded into the binary
OBJCOPY_FLAGS	?= -Obinary --set-section-flags .bss=alloc,load,contents

COMPILE		= $(COMPILER) $(DEBUGFLAGS)\
		  $(CFLAGS) $(ARCH_FLAGS) $(DEPFLAGS) $(INCLUDE_FLAGS)

LINT		= $(COMPILER) $(DEBUGFLAGS)\
		  $(CFLAGS) $(ARCH_FLAGS) $(LINTFLAGS) $(INCLUDE_FLAGS)

GENELF		= $(COMPILER) $(DEBUGFLAGS)\
		  $(CFLAGS) $(ARCH_FLAGS) $(LINKFLAGS) $(INCLUDE_FLAGS)

GENLINK		= $(COMPILER) $(PREPROCESS) $(DEPFLAGS) $(INCLUDE_FLAGS)
STRIPLINK	= sed -n '/^[^\#]/p'
KERN_SIZE	= wc -c kernel.bin | cut -d ' ' -f 1
KERN_INFO	= sed "s/<KERNEL_SIZE>/$$($(KERN_SIZE))/"

KERNEL_LINK	:= arch/$(ARCH)/conf/kernel-link
INIT_LINK	:= arch/$(ARCH)/conf/init-link

KERNEL_OBJECTS	!= ./scripts/gen-deps --compile "$(KERNEL_SOURCES)"
INIT_OBJECTS	!= ./scripts/gen-deps --compile "$(INIT_SOURCES)"
KERNEL_LD	!= ./scripts/gen-deps --link "$(KERNEL_LINK).S"
INIT_LD		!= ./scripts/gen-deps --link "$(INIT_LINK).S"

include deps.mk

$(INIT_LD): kernel.bin

lint: $(INIT_OBJECTS:.o=.o.l) $(KERNEL_OBJECTS:.o=.o.l)

init.elf: $(INIT_OBJECTS) $(INIT_LD)
	$(GENELF) -T $(INIT_LD) $(INIT_OBJECTS) -o $@

kernel.elf: $(KERNEL_OBJECTS) $(KERNEL_LD)
	$(GENELF) -T $(KERNEL_LD) $(KERNEL_OBJECTS) -o $@

init.bin: init.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

kernel.bin: kernel.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

apos.bin: init.bin kernel.bin
	cat init.bin kernel.bin > apos.bin

clean:
	$(RM) -r $(CLEANUP)

clean_run: clean
	$(CLEANUP_CMD)
