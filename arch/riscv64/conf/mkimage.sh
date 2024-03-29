#!/bin/sh

XLEN=$(echo "$1" | cut -c '6-7')

if [ ! -e fs ]; then
	mkdir fs
fi

if [ ! -e rootfs.img ]; then
	virt-make-fs --partition=mbr --type=vfat --size=10M fs rootfs.img
fi

SSIZE=$(fdisk -l rootfs.img | awk '$1=="Units:" {print $8}')
OFFSET=$(fdisk -l rootfs.img | awk '$1=="rootfs.img1" {print $2}')
mount -o loop,offset=$((${SSIZE}*${OFFSET})) rootfs.img fs

# copy over files outside the .itb to mess around with manual booting
cp kmi.bin fs
cp arch/riscv64/conf/initrd fs

# not entirely pleased with this solution, although eventually I should probably
# move `run` out of the kernel repo and into some `kmios` repo with a runtime
# and proper initrd etc. so this is good enough for now
../rv${XLEN}/u-boot-kmi/tools/mkimage -f arch/riscv${XLEN}/conf/kmi.its fs/kmi.itb
../rv${XLEN}/u-boot-kmi/tools/mkimage -A riscv -O kmi -T script \
	-d arch/riscv64/conf/boot.cmd fs/boot.scr

umount -l fs

qemu-system-riscv${XLEN} \
	-machine virt \
	-kernel ../rv${XLEN}/u-boot-kmi/u-boot \
	-bios ../rv${XLEN}/opensbi/build/platform/generic/firmware/fw_jump.elf \
	-device virtio-blk-device,drive=hd0 \
	-drive file=rootfs.img,format=raw,id=hd0 \
	-S -s -m 2G \
	-smp 2 \
	-serial stdio
