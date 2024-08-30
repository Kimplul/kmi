build/create/init.o.d:
-include build/create/init.o.d
build/create/init.o: create/
	mkdir -p build/create
	$(COMPILE_BENCHMARK) -c create/init.c -o build/create/init.o
build/create/initrd: build/create/init.o $(COMMON)
	$(COMPILE_BENCHMARK) build/create/init.o 	$(COMMON) -o build/create/init
	echo build/create/init | 	$(GEN_INITRD) build/create/initrd
BENCHMARKS += create
.PHONY: create
create: build/create/initrd
	mkdir -p reports/create
	timeout --foreground 30s 	$(QEMU) build/create/initrd > reports/create/log
build/fork/init.o.d:
-include build/fork/init.o.d
build/fork/init.o: fork/
	mkdir -p build/fork
	$(COMPILE_BENCHMARK) -c fork/init.c -o build/fork/init.o
build/fork/initrd: build/fork/init.o $(COMMON)
	$(COMPILE_BENCHMARK) build/fork/init.o 	$(COMMON) -o build/fork/init
	echo build/fork/init | 	$(GEN_INITRD) build/fork/initrd
BENCHMARKS += fork
.PHONY: fork
fork: build/fork/initrd
	mkdir -p reports/fork
	timeout --foreground 30s 	$(QEMU) build/fork/initrd > reports/fork/log
build/ipc-req/init.o.d:
-include build/ipc-req/init.o.d
build/ipc-req/init.o: ipc-req/
	mkdir -p build/ipc-req
	$(COMPILE_BENCHMARK) -c ipc-req/init.c -o build/ipc-req/init.o
build/ipc-req/initrd: build/ipc-req/init.o $(COMMON)
	$(COMPILE_BENCHMARK) build/ipc-req/init.o 	$(COMMON) -o build/ipc-req/init
	echo build/ipc-req/init | 	$(GEN_INITRD) build/ipc-req/initrd
BENCHMARKS += ipc-req
.PHONY: ipc-req
ipc-req: build/ipc-req/initrd
	mkdir -p reports/ipc-req
	timeout --foreground 30s 	$(QEMU) build/ipc-req/initrd > reports/ipc-req/log
build/malloc/init.o.d:
-include build/malloc/init.o.d
build/malloc/init.o: malloc/
	mkdir -p build/malloc
	$(COMPILE_BENCHMARK) -c malloc/init.c -o build/malloc/init.o
build/malloc/initrd: build/malloc/init.o $(COMMON)
	$(COMPILE_BENCHMARK) build/malloc/init.o 	$(COMMON) -o build/malloc/init
	echo build/malloc/init | 	$(GEN_INITRD) build/malloc/initrd
BENCHMARKS += malloc
.PHONY: malloc
malloc: build/malloc/initrd
	mkdir -p reports/malloc
	timeout --foreground 30s 	$(QEMU) build/malloc/initrd > reports/malloc/log
