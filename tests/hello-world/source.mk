TESTS	:= $(TESTS) hello-world

.PHONY: hello-world
hello-world: hello-world/init.c $(COMMON)
	mkdir -p build/hello-world
	$(COMPILE) hello-world/init.c build/printf.o -o build/hello-world/init
	echo build/hello-world/init | $(GEN_INITRD) build/hello-world/initrd
	$(QEMU) build/hello-world/initrd | grep 'Hello, world!'
