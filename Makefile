DO	!= echo > deps.mk
CFLAGS	:= -fno-pie -ggdb3 -ffreestanding -nostdlib
INCLUDE_DIRS	:= include

all: apos.bin

ARCH	?= riscv
CROSS_COMPILE ?= riscv64-unknown-elf-

# Common programs
CC	:= gcc
AR	:= ar
CPP	:= cpp
OBJCOPY	?= objcopy
OBJCOPY_FLAGS ?= -Obinary

KERNEL_SOURCES	:=
KERNEL_LINK	:=
INIT_SOURCES	:=
INIT_LINK	:=
INCLUDE_DIRS	:= include

include arch/$(ARCH)/source.mk

INCLUDE_FLAGS	!= ./scripts/prepend -I $(INCLUDE_DIRS)

KERNEL_OBJECTS	!= ./scripts/mkobjs $(CROSS_COMPILE)$(CC) $(CFLAGS)\
		$(INCLUDE_FLAGS) --kern-files $(KERNEL_SOURCES)

KERNEL_LD	!= ./scripts/mkobjs $(CROSS_COMPILE)$(CPP)\
		$(INCLUDE_FLAGS) --kern-lds $(KERNEL_LINK)

INIT_OBJECTS	!= ./scripts/mkobjs $(CROSS_COMPILE)$(CC) $(CFLAGS)\
		$(INCLUDE_FLAGS) --init-files $(INIT_SOURCES)

INIT_LD		!= ./scripts/mkobjs $(CROSS_COMPILE)$(CPP)\
		$(INCLUDE_FLAGS) --init-lds $(INIT_LINK)

include deps.mk

apos.bin: kernel.elf init.elf
	$(CROSS_COMPILE)$(OBJCOPY) $(OBJCOPY_FLAGS) kernel.elf kernel.bin
	$(CROSS_COMPILE)$(OBJCOPY) $(OBJCOPY_FLAGS) init.elf init.bin
	cat init.bin kernel.bin > $@

kernel.elf: $(KERNEL_OBJECTS) $(KERNEL_LD)
	$(CROSS_COMPILE)$(CC) -T$(KERNEL_LD) $(CFLAGS) $(INCLUDE_FLAGS) $(KERNEL_OBJECTS) -o $@

init.elf: $(INIT_OBJECTS) $(INIT_LD)
	$(CROSS_COMPILE)$(CC) -T$(INIT_LD) $(CFLAGS) $(INCLUDE_FLAGS) $(INIT_OBJECTS) -o $@

clean:
	rm $(KERNEL_OBJECTS) $(INIT_OBJECTS) $(INIT_LD) $(KERNEL_LD) kernel.bin kernel.elf init.bin init.elf apos.bin
