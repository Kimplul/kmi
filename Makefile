.PHONY: all
all: setup
	$(MAKE) -f scripts/makefile

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
	@./scripts/gen-deps -p INIT -c COMPILE_INIT -b init "${INIT_SOURCES}"

# default values, overwrite if/when needed
ARCH		?= riscv64
ARCH_SOURCE	= arch/$(ARCH)

# might consider renaming common, currently it refers to stuff common
# to all arches but clearly there are bits that are common to init and kernel
KERNEL_SOURCES	!= echo common/*.c common/uapi/*.c lib/*.c
INIT_SOURCES	!= echo lib/fdt*.c common/fdt.c common/string.c

CLEANUP		:= build deps.mk kernel.* init.* kmi.bin
CLEANUP_CMD	:=

include arch/$(ARCH)/source.mk

.PHONY: format
format:
	find arch lib common include -iname '*.[ch]' |\
		xargs uncrustify -c uncrustify.conf --no-backup -F -

.PHONY: license
license:
	find arch lib common include -iname '*.[ch]' |\
		xargs ./scripts/license

.PHONY: docs
docs:
	find arch lib common include -iname '*.[ch]' -not -path */gen/* |\
		xargs ./scripts/warn-undocumented
	doxygen docs/doxygen.conf

RM	= rm

.PHONY: clean
clean:
	$(RM) -rf $(CLEANUP)

.PHONY: clean_run
clean_run:
	$(CLEANUP_CMD)

.PHONY: clean_docs
clean_docs:
	$(RM) -rf docs/output

.PHONY: clean_all
clean_all: clean clean_run clean_docs
