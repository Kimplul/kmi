TESTS		:=

ARCH		?= riscv64
CROSS_COMPILE	?= $(ARCH)-unknown-elf-

LLVM		?= 0
COMPILER	!= [ "$(LLVM)" != "0" ] \
			&& echo clang --target="$(CROSS_COMPILE)" \
			|| echo $(CROSS_COMPILE)gcc

OBFLAGS		:= -ffreestanding -nostdlib -std=c17 -g
INCLUDEFLAGS	:= -I ../include -I.
WARNFLAGS	:= -Wall -Wextra
COMPILE		:= $(COMPILER) $(WARNFLAGS) $(INCLUDEFLAGS) $(OBFLAGS)

GEN_INITRD	:= cpio -H newc -o >
QEMU		:= qemu-system-$(ARCH) -machine virt -kernel ../kmi.bin \
		   -serial stdio \
		   -monitor none \
		   -nographic \
		   -no-reboot \
		   -initrd

COMMON		:= build/printf.o ../kmi.bin ../include/kmi/syscalls.h

include noop/source.mk
include hello-world/source.mk

.PHONY: check
check: $(TESTS)

build/printf.o: common/printf.c
	mkdir -p build
	$(COMPILE) -c common/printf.c -o build/printf.o

RM	?= rm

.PHONY: clean
clean:
	$(RM) -rf build
