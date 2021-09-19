DO		!= echo > deps.mk

# this could be done better
DEBUGFLAGS	!= [ $(DEBUG) ] && echo "-O0 -ggdb3 -DDEBUG" || echo "-flto -O2"
CFLAGS		= -fno-pie -ffreestanding -nostdlib -std=c11 -Wall -Wextra
DEPFLAGS	= -MT $@ -MMD -MP -MF $@.d

all: apos.bin

ARCH		?= riscv
CROSS_COMPILE	?= riscv64-unknown-elf-

# Common programs
CC		:= gcc
AR		:= ar
CPP		:= cpp
OBJCOPY		?= objcopy
OBJCOPY_FLAGS	?= -Obinary --set-section-flags .bss=alloc,load,contents

COMMON_SOURCES	!= echo common/*.c
KERNEL_SOURCES	!= echo kernel/*.c $(COMMON_SOURCES)
INIT_SOURCES	:= $(COMMON_SOURCES)
CLEANUP		:= build deps.mk kernel.* init.* apos.bin
CLEANUP_CMD	:=

include arch/$(ARCH)/source.mk

INCLUDE_FLAGS	:= -I include -I arch/$(ARCH)/include\
	-include config.h -include arch/$(ARCH)/config.h

COMPILE		= $(CROSS_COMPILE)$(CC) $(DEBUGFLAGS)\
		  $(CFLAGS) $(ARCH_FLAGS) $(DEPFLAGS) $(INCLUDE_FLAGS)

GENELF		= $(CROSS_COMPILE)$(CC) $(DEBUGFLAGS)\
		  $(CFLAGS) $(ARCH_FLAGS) $(INCLUDE_FLAGS)

GENLINK		= $(CROSS_COMPILE)$(CPP) $(DEPFLAGS) $(INCLUDE_FLAGS)
STRIPLINK	= sed -n '/^[^\#]/p'
KERN_SIZE	= wc -c kernel.bin | cut -d ' ' -f 1
KERN_INFO	= sed "s/<KERNEL_SIZE>/$$($(KERN_SIZE))/"

KERNEL_LINK	:= arch/$(ARCH)/conf/kernel-link
INIT_LINK	:= arch/$(ARCH)/conf/init-link

KERNEL_OBJECTS	!= ./scripts/gen-deps --kern "$(KERNEL_SOURCES)"
INIT_OBJECTS	!= ./scripts/gen-deps --init "$(INIT_SOURCES)"
KERNEL_LD	!= ./scripts/gen-deps --kern-link "$(KERNEL_LINK).S"
INIT_LD		!= ./scripts/gen-deps --init-link "$(INIT_LINK).S"

include deps.mk

apos.bin: kernel.bin init.bin
	cat init.bin kernel.bin > $@

kernel.elf: $(KERNEL_OBJECTS) $(KERNEL_LD)
	$(GENELF) -T $(KERNEL_LD) $(KERNEL_OBJECTS) -o $@

init.elf: kernel.bin $(INIT_OBJECTS) $(INIT_LD)
	$(GENELF) -T $(INIT_LD) $(INIT_OBJECTS) -o $@

kernel.bin: kernel.elf
	$(CROSS_COMPILE)$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

init.bin: init.elf
	$(CROSS_COMPILE)$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

clean:
	$(RM) -r $(CLEANUP)

clean_run: clean
	$(CLEANUP_CMD)
