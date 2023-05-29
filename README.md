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

This results in `kmi.bin`, which is the loader and the payload kernel.
The loader should support being loaded to arbitrary memory locations.
You should avoid the lowest 1 MiB of RAM, as the kernel is relocated thereabouts
after the loader has kicked itself into virtual memory.

Useful `make` flags and targets:

+ `docs`: Generate HTML documentation with doxygen. Currently mostly documents API stuff,
but my intention is to include higher level documentation as well.
Open `docs/output/html/index.html` in your favorite web browser.

+ `RELEASE=<0/1>`: Enable optimizations when set to `1`. Default is `0`.
Note that with `RELEASE=1` the built in serial driver is disabled, as eventually
I'd like to provide it as a separate userspace driver. At the moment only 8250 and
compatible serial devices are supported.

+ `LLVM=<0/1>`: Use LLVM toolchain when set to `1`. Default is `0`.
Note that due to some bugs in the toolchain, LTO is disabled with `RELEASE=1`.

+ `UBSAN=<0/1>`: Enable undefined behavior sanitizer, outputs a number of warnings at
runtime when undefined behavior is detected. Only available with `RELEASE=0`.

+ `GENERIC_UBOOT=<0/1>`: Compile for use with generic u-boot. This allows
booting the kernel through u-boot's `go` command. This would in theory allow
using essentially any precompiled u-boot as a bootloader, but requires some
extra effort by the user to manage loading different parts to where they should
go. I should probably come up with a full example, but approximately:

1. Load `kmi.bin` to address `A`
2. Load `initrd` to address `B`
3. Load FDT to address `C` (either provided by u-boot or a separate file)
4. Add `initrd` to FDT's `chosen` node
5. Boot with `go ${A} ${C}`.

Note that u-boot likes to skip the `0x` prefix for
hexadecimal values, which can confuse my string converter.
Check that the strings you pass to `go` have their correct prefixes.

+ `run`: Load a test program into `qemu` and run it. Requires some outside support at
the moment, please see [kmi-example](https://github.com/Kimplul/kmi-example).
This command might eventually be moved out of this repo.

+ `clean/clean_docs/clean_run/clean_all`: Clean compile artefacts, clean documentation
artefacts, clean run artefacts and clean all artefacts, respectively.
Note that `clean_run` and `clean_all` may need root privileges.

# Top-level view

This should eventually be moved into documentation with more details, but in short, `kmi`
is a hybrid kernel with thread migration as the main method of inter-process
communication. The kernel itself handles interrupts, memory management and thread
migration, but everything else is inteded to be up to the operating system built on top
of the kernel.

I chose thread migration as the main IPC method because it seemed interesting, somewhat
out of the ordinary and has some parallel features that I think might come in handy as
computers slowly but surely become more parallel.

I chose to implement memory management in the kernel mainly for ease of development.
While I am generally in favor of microkernels and separating responsibilities, I find
that memory is central enough to computers that trying to write a kernel without memory
management would be too difficult for me.

# Future plans

Interrupt and exception handling is still largely TODO.

To keep things interesting I'm working on a heavily simplified Unix-like operating system,
as of yet unreleased, that runs on the `kmi` kernel, with my main goals to provide a
shell environment and disk access. I'm hoping that having a somewhat usable 'full'
system will help me run into bugs more effectively and keep me more motivated to
work on the project.
