.PHONY: all
all: check

ARCH		?= riscv64
CROSS_COMPILE	?= $(ARCH)-unknown-elf-

LLVM		?= 0
COMPILER	!= [ "$(LLVM)" != "0" ] \
			&& echo clang --target="$(CROSS_COMPILE)" \
			|| echo $(CROSS_COMPILE)gcc

include common/arch/$(ARCH)/source.mk

OBFLAGS		:= -ffreestanding -nostdlib -std=c17 -g -O2
INCLUDEFLAGS	:= -I ../include -I.
WARNFLAGS	:= -Wall -Wextra
DEPFLAGS	= -MT $@ -MMD -MP -MF $@.d
COMPILE_TEST	= $(COMPILER) $(WARNFLAGS) $(INCLUDEFLAGS) \
		  $(DEPFLAGS) $(OBFLAGS) $(ARCH_FLAGS)

GEN_INITRD	:= cpio -H newc -o >
QEMU		:= qemu-system-$(ARCH) -machine virt -kernel ../kmi.bin \
		   -serial stdio \
		   -monitor none \
		   -nographic \
		   -no-reboot \
		   -m 128M \
		   -initrd

KMI		:= ../kmi.bin
COMMON		:= build/printf.o build/string.o

build/printf.o: common/printf.c $(KMI)
	$(COMPILE_TEST) -c common/printf.c -o build/printf.o

build/string.o: common/string.c $(KMI)
	$(COMPILE_TEST) -c common/string.c -o build/string.o

include tests.mk

.PHONY: check
check: $(TESTS)
	@cd reports; for d in * ; do						\
		if [ ! -f "$$d/log" ]; then					\
			echo "BROKEN: $$d" ;					\
		elif grep 'BUG|ERR' $$d/log					\
		     || [ "$$(tail -n1 $$d/log | tr -d '\r')" != "OK" ]; then	\
			echo "FAIL: $$d" ;					\
		fi								\
	done
	@echo "Done."
