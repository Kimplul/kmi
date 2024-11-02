.PHONY: all
all: benchmark

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
COMPILE_BENCHMARK	= $(COMPILER) $(WARNFLAGS) $(INCLUDEFLAGS) \
			  $(DEPFLAGS) $(OBFLAGS) $(ARCH_FLAGS)

GEN_INITRD	:= cpio -H newc -o >

# icount gives a rough approximation of how many instructions are executed, but
# *actual* performance would still have to be measured on real systems
QEMU		:= qemu-system-$(ARCH) -machine virt -kernel ../kmi.bin \
		   -serial stdio \
		   -monitor none \
		   -nographic \
		   -no-reboot \
		   -m 128M \
		   -icount 0 \
		   -initrd

KMI		:= ../kmi.bin
COMMON		:= build/printf.o build/string.o

build/printf.o: common/printf.c $(KMI)
	$(COMPILE_BENCHMARK) -c common/printf.c -o build/printf.o

build/string.o: common/string.c $(KMI)
	$(COMPILE_BENCHMARK) -c common/string.c -o build/string.o

BENCHMARKS :=
include benchmarks.mk

.PHONY: benchmark
benchmark: $(BENCHMARKS)
	@cd reports; for d in * ; do				\
		printf "%8s: " "$$d"; tail -n 1 $$d/log | tr -d '\r' | bc -l;	\
	done
