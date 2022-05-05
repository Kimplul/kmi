build/common/bits.o.d:
-include build/common/bits.o.d
build/common/bits.o: common/bits.c
	$(COMPILE) $(KERN_FLAGS) -c common/bits.c -o build/common/bits.o
build/common/bits.o.l: common/bits.c
	$(LINT) $(KERN_FLAGS) -c common/bits.c -o /dev/null
build/common/debug.o.d:
-include build/common/debug.o.d
build/common/debug.o: common/debug.c
	$(COMPILE) $(KERN_FLAGS) -c common/debug.c -o build/common/debug.o
build/common/debug.o.l: common/debug.c
	$(LINT) $(KERN_FLAGS) -c common/debug.c -o /dev/null
build/common/dmem.o.d:
-include build/common/dmem.o.d
build/common/dmem.o: common/dmem.c
	$(COMPILE) $(KERN_FLAGS) -c common/dmem.c -o build/common/dmem.o
build/common/dmem.o.l: common/dmem.c
	$(LINT) $(KERN_FLAGS) -c common/dmem.c -o /dev/null
build/common/elf.o.d:
-include build/common/elf.o.d
build/common/elf.o: common/elf.c
	$(COMPILE) $(KERN_FLAGS) -c common/elf.c -o build/common/elf.o
build/common/elf.o.l: common/elf.c
	$(LINT) $(KERN_FLAGS) -c common/elf.c -o /dev/null
build/common/fdt.o.d:
-include build/common/fdt.o.d
build/common/fdt.o: common/fdt.c
	$(COMPILE) $(KERN_FLAGS) -c common/fdt.c -o build/common/fdt.o
build/common/fdt.o.l: common/fdt.c
	$(LINT) $(KERN_FLAGS) -c common/fdt.c -o /dev/null
build/common/initrd.o.d:
-include build/common/initrd.o.d
build/common/initrd.o: common/initrd.c
	$(COMPILE) $(KERN_FLAGS) -c common/initrd.c -o build/common/initrd.o
build/common/initrd.o.l: common/initrd.c
	$(LINT) $(KERN_FLAGS) -c common/initrd.c -o /dev/null
build/common/main.o.d:
-include build/common/main.o.d
build/common/main.o: common/main.c
	$(COMPILE) $(KERN_FLAGS) -c common/main.c -o build/common/main.o
build/common/main.o.l: common/main.c
	$(LINT) $(KERN_FLAGS) -c common/main.c -o /dev/null
build/common/mem.o.d:
-include build/common/mem.o.d
build/common/mem.o: common/mem.c
	$(COMPILE) $(KERN_FLAGS) -c common/mem.c -o build/common/mem.o
build/common/mem.o.l: common/mem.c
	$(LINT) $(KERN_FLAGS) -c common/mem.c -o /dev/null
build/common/mem_nodes.o.d:
-include build/common/mem_nodes.o.d
build/common/mem_nodes.o: common/mem_nodes.c
	$(COMPILE) $(KERN_FLAGS) -c common/mem_nodes.c -o build/common/mem_nodes.o
build/common/mem_nodes.o.l: common/mem_nodes.c
	$(LINT) $(KERN_FLAGS) -c common/mem_nodes.c -o /dev/null
build/common/mem_regions.o.d:
-include build/common/mem_regions.o.d
build/common/mem_regions.o: common/mem_regions.c
	$(COMPILE) $(KERN_FLAGS) -c common/mem_regions.c -o build/common/mem_regions.o
build/common/mem_regions.o.l: common/mem_regions.c
	$(LINT) $(KERN_FLAGS) -c common/mem_regions.c -o /dev/null
build/common/nodes.o.d:
-include build/common/nodes.o.d
build/common/nodes.o: common/nodes.c
	$(COMPILE) $(KERN_FLAGS) -c common/nodes.c -o build/common/nodes.o
build/common/nodes.o.l: common/nodes.c
	$(LINT) $(KERN_FLAGS) -c common/nodes.c -o /dev/null
build/common/pmem.o.d:
-include build/common/pmem.o.d
build/common/pmem.o: common/pmem.c
	$(COMPILE) $(KERN_FLAGS) -c common/pmem.c -o build/common/pmem.o
build/common/pmem.o.l: common/pmem.c
	$(LINT) $(KERN_FLAGS) -c common/pmem.c -o /dev/null
build/common/proc.o.d:
-include build/common/proc.o.d
build/common/proc.o: common/proc.c
	$(COMPILE) $(KERN_FLAGS) -c common/proc.c -o build/common/proc.o
build/common/proc.o.l: common/proc.c
	$(LINT) $(KERN_FLAGS) -c common/proc.c -o /dev/null
build/common/sp_tree.o.d:
-include build/common/sp_tree.o.d
build/common/sp_tree.o: common/sp_tree.c
	$(COMPILE) $(KERN_FLAGS) -c common/sp_tree.c -o build/common/sp_tree.o
build/common/sp_tree.o.l: common/sp_tree.c
	$(LINT) $(KERN_FLAGS) -c common/sp_tree.c -o /dev/null
build/common/string.o.d:
-include build/common/string.o.d
build/common/string.o: common/string.c
	$(COMPILE) $(KERN_FLAGS) -c common/string.c -o build/common/string.o
build/common/string.o.l: common/string.c
	$(LINT) $(KERN_FLAGS) -c common/string.c -o /dev/null
build/common/tcb.o.d:
-include build/common/tcb.o.d
build/common/tcb.o: common/tcb.c
	$(COMPILE) $(KERN_FLAGS) -c common/tcb.c -o build/common/tcb.o
build/common/tcb.o.l: common/tcb.c
	$(LINT) $(KERN_FLAGS) -c common/tcb.c -o /dev/null
build/common/timer.o.d:
-include build/common/timer.o.d
build/common/timer.o: common/timer.c
	$(COMPILE) $(KERN_FLAGS) -c common/timer.c -o build/common/timer.o
build/common/timer.o.l: common/timer.c
	$(LINT) $(KERN_FLAGS) -c common/timer.c -o /dev/null
build/common/vmem.o.d:
-include build/common/vmem.o.d
build/common/vmem.o: common/vmem.c
	$(COMPILE) $(KERN_FLAGS) -c common/vmem.c -o build/common/vmem.o
build/common/vmem.o.l: common/vmem.c
	$(LINT) $(KERN_FLAGS) -c common/vmem.c -o /dev/null
build/common/uapi/conf.o.d:
-include build/common/uapi/conf.o.d
build/common/uapi/conf.o: common/uapi/conf.c
	$(COMPILE) $(KERN_FLAGS) -c common/uapi/conf.c -o build/common/uapi/conf.o
build/common/uapi/conf.o.l: common/uapi/conf.c
	$(LINT) $(KERN_FLAGS) -c common/uapi/conf.c -o /dev/null
build/common/uapi/dispatch.o.d:
-include build/common/uapi/dispatch.o.d
build/common/uapi/dispatch.o: common/uapi/dispatch.c
	$(COMPILE) $(KERN_FLAGS) -c common/uapi/dispatch.c -o build/common/uapi/dispatch.o
build/common/uapi/dispatch.o.l: common/uapi/dispatch.c
	$(LINT) $(KERN_FLAGS) -c common/uapi/dispatch.c -o /dev/null
build/common/uapi/ipc.o.d:
-include build/common/uapi/ipc.o.d
build/common/uapi/ipc.o: common/uapi/ipc.c
	$(COMPILE) $(KERN_FLAGS) -c common/uapi/ipc.c -o build/common/uapi/ipc.o
build/common/uapi/ipc.o.l: common/uapi/ipc.c
	$(LINT) $(KERN_FLAGS) -c common/uapi/ipc.c -o /dev/null
build/common/uapi/mem.o.d:
-include build/common/uapi/mem.o.d
build/common/uapi/mem.o: common/uapi/mem.c
	$(COMPILE) $(KERN_FLAGS) -c common/uapi/mem.c -o build/common/uapi/mem.o
build/common/uapi/mem.o.l: common/uapi/mem.c
	$(LINT) $(KERN_FLAGS) -c common/uapi/mem.c -o /dev/null
build/common/uapi/proc.o.d:
-include build/common/uapi/proc.o.d
build/common/uapi/proc.o: common/uapi/proc.c
	$(COMPILE) $(KERN_FLAGS) -c common/uapi/proc.c -o build/common/uapi/proc.o
build/common/uapi/proc.o.l: common/uapi/proc.c
	$(LINT) $(KERN_FLAGS) -c common/uapi/proc.c -o /dev/null
build/common/uapi/timers.o.d:
-include build/common/uapi/timers.o.d
build/common/uapi/timers.o: common/uapi/timers.c
	$(COMPILE) $(KERN_FLAGS) -c common/uapi/timers.c -o build/common/uapi/timers.o
build/common/uapi/timers.o.l: common/uapi/timers.c
	$(LINT) $(KERN_FLAGS) -c common/uapi/timers.c -o /dev/null
build/lib/fdt.o.d:
-include build/lib/fdt.o.d
build/lib/fdt.o: lib/fdt.c
	$(COMPILE) $(KERN_FLAGS) -c lib/fdt.c -o build/lib/fdt.o
build/lib/fdt.o.l: lib/fdt.c
	$(LINT) $(KERN_FLAGS) -c lib/fdt.c -o /dev/null
build/lib/fdt_addresses.o.d:
-include build/lib/fdt_addresses.o.d
build/lib/fdt_addresses.o: lib/fdt_addresses.c
	$(COMPILE) $(KERN_FLAGS) -c lib/fdt_addresses.c -o build/lib/fdt_addresses.o
build/lib/fdt_addresses.o.l: lib/fdt_addresses.c
	$(LINT) $(KERN_FLAGS) -c lib/fdt_addresses.c -o /dev/null
build/lib/fdt_dbg.o.d:
-include build/lib/fdt_dbg.o.d
build/lib/fdt_dbg.o: lib/fdt_dbg.c
	$(COMPILE) $(KERN_FLAGS) -c lib/fdt_dbg.c -o build/lib/fdt_dbg.o
build/lib/fdt_dbg.o.l: lib/fdt_dbg.c
	$(LINT) $(KERN_FLAGS) -c lib/fdt_dbg.c -o /dev/null
build/lib/fdt_empty_tree.o.d:
-include build/lib/fdt_empty_tree.o.d
build/lib/fdt_empty_tree.o: lib/fdt_empty_tree.c
	$(COMPILE) $(KERN_FLAGS) -c lib/fdt_empty_tree.c -o build/lib/fdt_empty_tree.o
build/lib/fdt_empty_tree.o.l: lib/fdt_empty_tree.c
	$(LINT) $(KERN_FLAGS) -c lib/fdt_empty_tree.c -o /dev/null
build/lib/fdt_ro.o.d:
-include build/lib/fdt_ro.o.d
build/lib/fdt_ro.o: lib/fdt_ro.c
	$(COMPILE) $(KERN_FLAGS) -c lib/fdt_ro.c -o build/lib/fdt_ro.o
build/lib/fdt_ro.o.l: lib/fdt_ro.c
	$(LINT) $(KERN_FLAGS) -c lib/fdt_ro.c -o /dev/null
build/lib/fdt_rw.o.d:
-include build/lib/fdt_rw.o.d
build/lib/fdt_rw.o: lib/fdt_rw.c
	$(COMPILE) $(KERN_FLAGS) -c lib/fdt_rw.c -o build/lib/fdt_rw.o
build/lib/fdt_rw.o.l: lib/fdt_rw.c
	$(LINT) $(KERN_FLAGS) -c lib/fdt_rw.c -o /dev/null
build/lib/fdt_strerror.o.d:
-include build/lib/fdt_strerror.o.d
build/lib/fdt_strerror.o: lib/fdt_strerror.c
	$(COMPILE) $(KERN_FLAGS) -c lib/fdt_strerror.c -o build/lib/fdt_strerror.o
build/lib/fdt_strerror.o.l: lib/fdt_strerror.c
	$(LINT) $(KERN_FLAGS) -c lib/fdt_strerror.c -o /dev/null
build/lib/fdt_sw.o.d:
-include build/lib/fdt_sw.o.d
build/lib/fdt_sw.o: lib/fdt_sw.c
	$(COMPILE) $(KERN_FLAGS) -c lib/fdt_sw.c -o build/lib/fdt_sw.o
build/lib/fdt_sw.o.l: lib/fdt_sw.c
	$(LINT) $(KERN_FLAGS) -c lib/fdt_sw.c -o /dev/null
build/lib/fdt_wip.o.d:
-include build/lib/fdt_wip.o.d
build/lib/fdt_wip.o: lib/fdt_wip.c
	$(COMPILE) $(KERN_FLAGS) -c lib/fdt_wip.c -o build/lib/fdt_wip.o
build/lib/fdt_wip.o.l: lib/fdt_wip.c
	$(LINT) $(KERN_FLAGS) -c lib/fdt_wip.c -o /dev/null
build/lib/ubsan.o.d:
-include build/lib/ubsan.o.d
build/lib/ubsan.o: lib/ubsan.c
	$(COMPILE) $(KERN_FLAGS) -c lib/ubsan.c -o build/lib/ubsan.o
build/lib/ubsan.o.l: lib/ubsan.c
	$(LINT) $(KERN_FLAGS) -c lib/ubsan.c -o /dev/null
build/arch/riscv64/kernel/cpu.o.d:
-include build/arch/riscv64/kernel/cpu.o.d
build/arch/riscv64/kernel/cpu.o: arch/riscv64/kernel/cpu.c
	$(COMPILE) $(KERN_FLAGS) -c arch/riscv64/kernel/cpu.c -o build/arch/riscv64/kernel/cpu.o
build/arch/riscv64/kernel/cpu.o.l: arch/riscv64/kernel/cpu.c
	$(LINT) $(KERN_FLAGS) -c arch/riscv64/kernel/cpu.c -o /dev/null
build/arch/riscv64/kernel/head.o.d:
-include build/arch/riscv64/kernel/head.o.d
build/arch/riscv64/kernel/head.o: arch/riscv64/kernel/head.S
	$(COMPILE) $(KERN_FLAGS) -c arch/riscv64/kernel/head.S -o build/arch/riscv64/kernel/head.o
build/arch/riscv64/kernel/head.o.l: arch/riscv64/kernel/head.S
	$(LINT) $(KERN_FLAGS) -c arch/riscv64/kernel/head.S -o /dev/null
build/arch/riscv64/kernel/irq.o.d:
-include build/arch/riscv64/kernel/irq.o.d
build/arch/riscv64/kernel/irq.o: arch/riscv64/kernel/irq.c
	$(COMPILE) $(KERN_FLAGS) -c arch/riscv64/kernel/irq.c -o build/arch/riscv64/kernel/irq.o
build/arch/riscv64/kernel/irq.o.l: arch/riscv64/kernel/irq.c
	$(LINT) $(KERN_FLAGS) -c arch/riscv64/kernel/irq.c -o /dev/null
build/arch/riscv64/kernel/main.o.d:
-include build/arch/riscv64/kernel/main.o.d
build/arch/riscv64/kernel/main.o: arch/riscv64/kernel/main.c
	$(COMPILE) $(KERN_FLAGS) -c arch/riscv64/kernel/main.c -o build/arch/riscv64/kernel/main.o
build/arch/riscv64/kernel/main.o.l: arch/riscv64/kernel/main.c
	$(LINT) $(KERN_FLAGS) -c arch/riscv64/kernel/main.c -o /dev/null
build/arch/riscv64/kernel/pmem.o.d:
-include build/arch/riscv64/kernel/pmem.o.d
build/arch/riscv64/kernel/pmem.o: arch/riscv64/kernel/pmem.c
	$(COMPILE) $(KERN_FLAGS) -c arch/riscv64/kernel/pmem.c -o build/arch/riscv64/kernel/pmem.o
build/arch/riscv64/kernel/pmem.o.l: arch/riscv64/kernel/pmem.c
	$(LINT) $(KERN_FLAGS) -c arch/riscv64/kernel/pmem.c -o /dev/null
build/arch/riscv64/kernel/power.o.d:
-include build/arch/riscv64/kernel/power.o.d
build/arch/riscv64/kernel/power.o: arch/riscv64/kernel/power.c
	$(COMPILE) $(KERN_FLAGS) -c arch/riscv64/kernel/power.c -o build/arch/riscv64/kernel/power.o
build/arch/riscv64/kernel/power.o.l: arch/riscv64/kernel/power.c
	$(LINT) $(KERN_FLAGS) -c arch/riscv64/kernel/power.c -o /dev/null
build/arch/riscv64/kernel/proc.o.d:
-include build/arch/riscv64/kernel/proc.o.d
build/arch/riscv64/kernel/proc.o: arch/riscv64/kernel/proc.c
	$(COMPILE) $(KERN_FLAGS) -c arch/riscv64/kernel/proc.c -o build/arch/riscv64/kernel/proc.o
build/arch/riscv64/kernel/proc.o.l: arch/riscv64/kernel/proc.c
	$(LINT) $(KERN_FLAGS) -c arch/riscv64/kernel/proc.c -o /dev/null
build/arch/riscv64/kernel/sbi.o.d:
-include build/arch/riscv64/kernel/sbi.o.d
build/arch/riscv64/kernel/sbi.o: arch/riscv64/kernel/sbi.c
	$(COMPILE) $(KERN_FLAGS) -c arch/riscv64/kernel/sbi.c -o build/arch/riscv64/kernel/sbi.o
build/arch/riscv64/kernel/sbi.o.l: arch/riscv64/kernel/sbi.c
	$(LINT) $(KERN_FLAGS) -c arch/riscv64/kernel/sbi.c -o /dev/null
build/arch/riscv64/kernel/timer.o.d:
-include build/arch/riscv64/kernel/timer.o.d
build/arch/riscv64/kernel/timer.o: arch/riscv64/kernel/timer.c
	$(COMPILE) $(KERN_FLAGS) -c arch/riscv64/kernel/timer.c -o build/arch/riscv64/kernel/timer.o
build/arch/riscv64/kernel/timer.o.l: arch/riscv64/kernel/timer.c
	$(LINT) $(KERN_FLAGS) -c arch/riscv64/kernel/timer.c -o /dev/null
build/arch/riscv64/kernel/vmem.o.d:
-include build/arch/riscv64/kernel/vmem.o.d
build/arch/riscv64/kernel/vmem.o: arch/riscv64/kernel/vmem.c
	$(COMPILE) $(KERN_FLAGS) -c arch/riscv64/kernel/vmem.c -o build/arch/riscv64/kernel/vmem.o
build/arch/riscv64/kernel/vmem.o.l: arch/riscv64/kernel/vmem.c
	$(LINT) $(KERN_FLAGS) -c arch/riscv64/kernel/vmem.c -o /dev/null
build/arch/riscv64/init/init.o.d:
-include build/arch/riscv64/init/init.o.d
build/arch/riscv64/init/init.o: arch/riscv64/init/init.c
	$(COMPILE) $(INIT_FLAGS) -c arch/riscv64/init/init.c -o build/arch/riscv64/init/init.o
build/arch/riscv64/init/init.o.l: arch/riscv64/init/init.c
	$(LINT) $(INIT_FLAGS) -c arch/riscv64/init/init.c -o /dev/null
build/arch/riscv64/init/start.o.d:
-include build/arch/riscv64/init/start.o.d
build/arch/riscv64/init/start.o: arch/riscv64/init/start.S
	$(COMPILE) $(INIT_FLAGS) -c arch/riscv64/init/start.S -o build/arch/riscv64/init/start.o
build/arch/riscv64/init/start.o.l: arch/riscv64/init/start.S
	$(LINT) $(INIT_FLAGS) -c arch/riscv64/init/start.S -o /dev/null
build/arch/riscv64/conf/kernel-link.ld.d:
-include build/arch/riscv64/conf/kernel-link.ld.d
build/arch/riscv64/conf/kernel-link.ld: arch/riscv64/conf/kernel-link.S
	$(GENLINK) arch/riscv64/conf/kernel-link.S | $(STRIPLINK) > build/arch/riscv64/conf/kernel-link.ld
build/arch/riscv64/conf/init-link.ld.d:
-include build/arch/riscv64/conf/init-link.ld.d
build/arch/riscv64/conf/init-link.ld: arch/riscv64/conf/init-link.S
	$(GENLINK) arch/riscv64/conf/init-link.S | $(STRIPLINK) | $(KERN_INFO) > build/arch/riscv64/conf/init-link.ld
