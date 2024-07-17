check:
	$(MAKE) -f scripts/makefile check

.DEFAULT:
	$(MAKE) -f scripts/makefile $<

SOURCES		!= echo */source.mk
DO		!= echo -n > tests.mk
include $(SOURCES)

build/printf.o: common/printf.c
	mkdir -p build && $(COMPILE_TEST) -c common/printf.c -o build/printf.o

RM	?= rm

.PHONY: clean
clean:
	$(RM) -rf build reports
