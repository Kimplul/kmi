.PHONY: benchmark
benchmark:
	$(MAKE) -f scripts/makefile benchmark

.DEFAULT:
	$(MAKE) -f scripts/makefile $<

SOURCES		!= echo */source.mk
DO		!= echo -n > benchmarks.mk
include $(SOURCES)

RM	?= rm

.PHONY: clean
clean:
	$(RM) -rf build
