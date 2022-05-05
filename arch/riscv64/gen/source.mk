OFFSET_HEADER	:= $(ARCH_SOURCE)/include/gen/asm-offsets.h
OFFSET_SOURCE	:= $(ARCH_SOURCE)/gen/asm-offsets.c

$(OFFSET_HEADER): $(OFFSET_SOURCE)
	echo "#ifndef APOS_ASM_OFFSETS_H" > $(OFFSET_HEADER)
	echo "#define APOS_ASM_OFFSETS_H" >> $(OFFSET_HEADER)
	$(COMPILER) $(DEPFLAGS) $(INCLUDE_FLAGS) -S $(OFFSET_SOURCE) -o - |\
		awk '($$1 == "#->") { print "#define " $$2 " " $$3 }' >> $(OFFSET_HEADER)
	echo "#endif /* APOS_ASM_OFFSETS_H */" >> $(OFFSET_HEADER)

CLEANUP	+= $(ARCH_SOURCE)/include/gen/*
