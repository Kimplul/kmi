Requirements for a new arch:
+ arch/$(ARCH)/conf/init-link.S
+ arch/$(ARCH)/conf/init-kernel.S
+ arch/$(ARCH)/source.mk
	-> populate KERNEL_SOURCES, INIT_SOURCES and CLEANUP if required
