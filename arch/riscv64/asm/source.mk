OFFSET_HEADER	:= $(ARCH_SOURCE)/kernel/gen/asm-offsets.h
OFFSET_SOURCE	:= $(ARCH_SOURCE)/asm/asm-offsets.c

$(OFFSET_HEADER): $(OFFSET_SOURCE)
	mkdir -p $$(dirname $(OFFSET_HEADER))
	echo "#ifndef KMI_ASM_OFFSETS_H" > $(OFFSET_HEADER)
	echo "#define KMI_ASM_OFFSETS_H" >> $(OFFSET_HEADER)
	echo "/**" >> $(OFFSET_HEADER)
	echo " * @file asm-offsets.h" >> $(OFFSET_HEADER)
	echo " * This comment is to shut up warnings." >> $(OFFSET_HEADER)
	echo " */" >> $(OFFSET_HEADER)
	$(COMPILER) $(CFLAGS) $(INCLUDE_FLAGS) -S $(OFFSET_SOURCE) -o - |\
		awk '($$1 == "#->") { print "#define " $$2 " " $$3 }' >> $(OFFSET_HEADER)
	echo "#endif /* KMI_ASM_OFFSETS_H */" >> $(OFFSET_HEADER)

CLEANUP	+= $(ARCH_SOURCE)/kernel/gen
