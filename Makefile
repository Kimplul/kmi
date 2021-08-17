CFLAGS := -fno-pie -g -ffreestanding -nostdlib
INCLUDE_DIRS := include

all: kernel.bin

KERNEL_SOURCES :=
INIT_SOURCES :=
LINKER_SOURCE :=
TARGET ?= riscv
CROSS_COMPILER ?= riscv64-unknown-elf-
CC := gcc
AR := ar
CPP := cpp
OBJCOPY := objcopy
OBJCOPY_FLAGS := -Obinary

include arch/source.mk

INCLUDE_FLAGS := $(addprefix -I,$(INCLUDE_DIRS))
KERNEL_OBJECTS := $(patsubst %.c,%.o,$(KERNEL_SOURCES))
KERNEL_OBJECTS := $(patsubst %.S,%.o,$(KERNEL_OBJECTS))

LINKER_FILE := $(patsubst %.S,%.ld,$(LINKER_SOURCE))

kernel.bin: kernel.elf
	$(CROSS_COMPILER)$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

kernel.elf: $(KERNEL_OBJECTS) | $(LINKER_FILE)
	$(CROSS_COMPILER)$(CC) -T$(LINKER_FILE) $(CFLAGS) $(INCLUDE_FLAGS) $^ -o $@

%.o: %.c
	$(CROSS_COMPILER)$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

%.o: %.S
	$(CROSS_COMPILER)$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

$(LINKER_FILE): $(LINKER_SOURCE)
	$(CROSS_COMPILER)$(CPP) $(INCLUDE_FLAGS) $< | sed -n '/^[^#]/p' > $@

clean:
	rm $(KERNEL_OBJECTS) $(INIT_OBJECTS) $(LINKER_FILE) kernel.bin kernel.elf
