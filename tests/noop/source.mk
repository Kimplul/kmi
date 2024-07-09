TESTS	:= $(TESTS) noop

.PHONY: noop
noop: noop/init.c $(COMMON)
	mkdir -p build/noop
	$(COMPILE) noop/init.c build/printf.o -o build/noop/init
	echo build/noop/init | $(GEN_INITRD) build/noop/initrd
	$(QEMU) build/noop/initrd | grep 'OK'
