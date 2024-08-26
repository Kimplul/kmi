.PHONY: all
all: check

ARCH		?= riscv64
CROSS_COMPILE	?= $(ARCH)-unknown-elf-

LLVM		?= 0
COMPILER	!= [ "$(LLVM)" != "0" ] \
			&& echo clang --target="$(CROSS_COMPILE)" \
			|| echo $(CROSS_COMPILE)gcc

OBFLAGS		:= -ffreestanding -nostdlib -std=c17 -g
INCLUDEFLAGS	:= -I ../include -I.
WARNFLAGS	:= -Wall -Wextra
DEPFLAGS	= -MT $@ -MMD -MP -MF $@.d
COMPILE_TEST	= $(COMPILER) $(WARNFLAGS) $(INCLUDEFLAGS) $(DEPFLAGS) $(OBFLAGS)

GEN_INITRD	:= cpio -H newc -o >
QEMU		:= qemu-system-$(ARCH) -machine virt -kernel ../kmi.bin \
		   -serial stdio \
		   -monitor none \
		   -nographic \
		   -no-reboot \
		   -m 128M \
		   -initrd

KMI		:= ../kmi.bin
COMMON		:= build/printf.o

build/printf.o: common/printf.c $(KMI)
	$(COMPILE_TEST) -c common/printf.c -o build/printf.o

include tests.mk

.PHONY: check
check: $(TESTS)
	@cd reports; for d in * ; do				\
		if [ ! -f "$$d/OK" ]; then			\
			echo "BROKEN: $$d" ;			\
		elif [ "$$(tail -n1 $$d/OK)" != "OK" ]; then	\
			echo "FAIL: $$d" ;			\
		fi						\
	done
	@echo "Done."
