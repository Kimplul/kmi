# `kmi`

`kmi` is a ***K***ernel with ***MI***grating threads I've been writing as a hobby project.
To see it in action, have a look at [kmi-example](https://github.com/Kimplul/kmi-example).

Currently the kernel only works in `qemu` with RISC-V 64 bit. There is some code for
RISC-V 32 bit support, but at the moment I'm focusing on 64 bit and 32 is less tested.

# Building

`riscv64-unknown-elf-` is the default toolchain looked for by the build system, but
`riscv64-linux-gnu-` will also work. Either should be available from your package manager.

Simple build with default configuration:
+ `make`

This results in `kmi.bin`, which is the bootloader and the payload kernel.
Currently the kernel expects to be loaded into a specific address, allowing arbitrary boot
locations is on my TODO list. The load address depends on the configured RAM base address,
see `arch/riscv64/conf/kmi.its` and `PM_KERN_BASE`. Dynamically detecting RAM base is also TODO.

Useful `make` flags and targets:

+ `docs`: Generate HTML documentation with doxygen. Currently mostly documents API stuff,
but my intention is to include higher level documentation as well.
Open `docs/output/html/index.html` in your favorite web browser.

+ `RELEASE=<0/1>`: Enable optimizations when set to `1`. Default is `0`.
Note that with `RELEASE=1` the built in serial driver is disabled, as eventually
I'd like to provide it as a separate userspace driver. At the moment only NS16550A and
compatible serial devices are supported.

+ `LLVM=<0/1>`: Use LLVM toolchain when set to `1`. Default is `0`.
Note that due to some bugs and missing features in LLVM RISC-V support, some GNU
binutils tools are still needed, mainly `objcopy`. Also, when combined with `RELEASE`,
disables LTO as `lld` seemed to have issues with some relaxed instructions with LTO enabled.

+ `UBSAN=<0/1>`: Enable undefined behavior sanitizer, outputs a number of warnings at runtime when
undefined behavior is detected. Only available with `RELEASE=0`.

+ `run`: Load a test program into `qemu` and run it. Requires some outside support at the moment,
please see [kmi-example](https://github.com/Kimplul/kmi-example). This command might eventually be moved
out of this repo.

+ `clean/clean_docs/clean_run/clean_all`: Clean compile artefacts, clean documentation artefacts,
clean run artefacts and clean all artefacts, respectively. Note that `clean_run` and `clean_all` may
need root privileges.
