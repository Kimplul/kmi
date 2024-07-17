hello-world_OBJS += build/hello-world/init.o
build/hello-world/init.o.d:
-include build/hello-world/init.o.d
build/hello-world/init.o: hello-world/init.c
	$(COMPILE_TEST) -c hello-world/init.c -o build/hello-world/init.o
build/hello-world/init: $(hello-world_OBJS) $(COMMON)
	$(COMPILE_TEST) $(hello-world_OBJS) $(COMMON) 		-o build/hello-world/init
include hello-world/check.mk
build/hello-world/initrd: build/hello-world/init $(COMMON) $(KMI)
	echo build/hello-world/init | $(GEN_INITRD) build/hello-world/initrd
reports/hello-world/log: build/hello-world/initrd
	mkdir -p reports/hello-world
	rm -f reports/hello-world/*
	timeout --foreground 30s $(QEMU) 	build/hello-world/initrd > reports/hello-world/log
TESTS += hello-world
.PHONY: do-hello-world
do-hello-world: reports/hello-world/log
noop_OBJS += build/noop/init.o
build/noop/init.o.d:
-include build/noop/init.o.d
build/noop/init.o: noop/init.c
	$(COMPILE_TEST) -c noop/init.c -o build/noop/init.o
build/noop/init: $(noop_OBJS) $(COMMON)
	$(COMPILE_TEST) $(noop_OBJS) $(COMMON) 		-o build/noop/init
include noop/check.mk
build/noop/initrd: build/noop/init $(COMMON) $(KMI)
	echo build/noop/init | $(GEN_INITRD) build/noop/initrd
reports/noop/log: build/noop/initrd
	mkdir -p reports/noop
	rm -f reports/noop/*
	timeout --foreground 30s $(QEMU) 	build/noop/initrd > reports/noop/log
TESTS += noop
.PHONY: do-noop
do-noop: reports/noop/log
