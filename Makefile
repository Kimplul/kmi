.PHONY: all
all: setup
	$(MAKE) -f scripts/makefile

.PHONY: check
check: all
	$(MAKE) -C tests check

.PHONY: benchmark
benchmark: all
	$(MAKE) -C benchmarks benchmark

# this kicks all unrecognised targets to the client script.
# note that trying to compile individual files, e.g.
#
#	make kernel.elf
#
# will not work, you would need
#
#	make -f scripts/makefile kernel.elf
#
# instead
.DEFAULT: setup
	$(MAKE) -f scripts/makefile $<

.PHONY:
setup:
	@echo -n > deps.mk
	@./scripts/gen-deps -p KERNEL -c COMPILE_KERNEL -b kernel "${KERNEL_SOURCES}"

# default values, overwrite if/when needed
ARCH		?= riscv64
ARCH_SOURCE	= arch/$(ARCH)

KERNEL_SOURCES	!= echo src/*.c src/uapi/*.c lib/*.c

CLEANUP		:= build deps.mk kmi.elf kmi.bin
CLEANUP_CMD	:=

include arch/$(ARCH)/source.mk

.PHONY: format
format:
	find arch lib src include -iname '*.[ch]' |\
		xargs uncrustify -c uncrustify.conf --no-backup -F -

.PHONY: license
license:
	find arch lib src include -iname '*.[ch]' |\
		xargs ./scripts/license

.PHONY: docs
docs:
	find arch lib src include -iname '*.[ch]' -not -path */gen/* |\
		xargs ./scripts/warn-undocumented
	doxygen docs/doxygen.conf

RM	= rm

.PHONY: clean
clean:
	$(MAKE) -C tests clean
	$(RM) -rf $(CLEANUP)

.PHONY: clean_tests
clean_tests:
	$(MAKE) -C tests clean

.PHONY: clean_benchmarks
clean_benchmarks:
	$(MAKE) -C benchmarks clean

.PHONY: clean_run
clean_run:
	$(CLEANUP_CMD)

.PHONY: clean_docs
clean_docs:
	$(RM) -rf docs/output

.PHONY: clean_all
clean_all: clean clean_run clean_docs
