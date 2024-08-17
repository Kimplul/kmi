check:
	$(MAKE) -f scripts/makefile check

.DEFAULT:
	$(MAKE) -f scripts/makefile $<

SOURCES		!= echo */source.mk
DO		!= echo -n > tests.mk
include $(SOURCES)

RM	?= rm

.PHONY: clean
clean:
	$(RM) -rf build reports
