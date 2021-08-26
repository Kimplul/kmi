DO		!= echo > deps.mk

DEBUGFLAGS	!= [ $(DEBUG) ] && echo "-g3 -DDEBUG" || echo "-O2"
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
OBJCOPY_FLAGS	?= -Obinary

COMMON_SOURCES	!= echo common/*.c
KERNEL_SOURCES	:= $(COMMON_SOURCES)
INIT_SOURCES	:= $(COMMON_SOURCES)
CLEANUP		:= build deps.mk kernel.* init.* apos.bin
CLEANUP_CMD	:=

include arch/$(ARCH)/source.mk

INCLUDE_FLAGS	:= -I include -I arch/$(ARCH)/include\
	-include config.h -include arch/$(ARCH)/config.h

COMPILE		= $(CROSS_COMPILE)$(CC) $(DEBUGFLAGS) $(CFLAGS) $(ARCH_FLAGS) $(DEPFLAGS) $(INCLUDE_FLAGS)
GENELF		= $(CROSS_COMPILE)$(CC) $(DEBUGFLAGS) $(CFLAGS) $(ARCH_FLAGS) $(INCLUDE_FLAGS)
GENLINK		= $(CROSS_COMPILE)$(CPP) $(DEPFLAGS) $(INCLUDE_FLAGS)
STRIPLINK	= sed -n '/^[^\#]/p'

KERNEL_LINK	:= arch/$(ARCH)/conf/kernel-link
INIT_LINK	:= arch/$(ARCH)/conf/init-link

KERNEL_OBJECTS	!= ./scripts/gen-deps --kern "$(KERNEL_SOURCES)"
INIT_OBJECTS	!= ./scripts/gen-deps --init "$(INIT_SOURCES)"
KERNEL_LD	!= ./scripts/gen-deps --link "$(KERNEL_LINK).S"
INIT_LD		!= ./scripts/gen-deps --link "$(INIT_LINK).S"

include deps.mk

apos.bin: kernel.elf init.elf
	$(CROSS_COMPILE)$(OBJCOPY) $(OBJCOPY_FLAGS) kernel.elf kernel.bin
	$(CROSS_COMPILE)$(OBJCOPY) $(OBJCOPY_FLAGS) init.elf init.bin
	cat init.bin kernel.bin > $@

kernel.elf: $(KERNEL_OBJECTS) $(KERNEL_LD)
	$(GENELF) -T $(KERNEL_LD) $(KERNEL_OBJECTS) -o $@

init.elf: $(INIT_OBJECTS) $(INIT_LD)
	$(GENELF) -T $(INIT_LD) $(INIT_OBJECTS) -o $@

clean:
	$(CLEANUP_CMD)
	$(RM) -r $(CLEANUP)
