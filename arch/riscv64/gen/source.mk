$(ARCH_SOURCE)/include/gen/asm-offsets.h: $(ARCH_SOURCE)/gen/asm-offsets.c
	echo "#ifndef APOS_ASM_OFFSETS_H" > $@
	echo "#define APOS_ASM_OFFSETS_H" >> $@
	$(COMPILER) $(DEPFLAGS) $(INCLUDE_FLAGS) -S $< -o - |\
		awk '($$1 == "#->") { print "#define " $$2 " " $$3 }' >> $@
	echo "#endif /* APOS_ASM_OFFSETS_H */" >> $@

CLEANUP	+= $(ARCH_SOURCE)/include/gen/*
