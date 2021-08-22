#!/bin/sh

if [ ! -e fs ]; then
	mkdir fs
fi

if [ ! -e rootfs.img ]; then
	virt-make-fs --partition=mbr --type=vfat --size=10M fs rootfs.img
fi


SSIZE=$(fdisk -l rootfs.img | awk '$1=="Units:" {print $8}')
OFFSET=$(fdisk -l rootfs.img | awk '$1=="rootfs.img1" {print $2}')
mount -o loop,offset=$((${SSIZE}*${OFFSET})) rootfs.img fs

# not entirely pleased with this solution, although eventually I should probably
# move `run` out of the kernel repo and into some `aposos` repo with a runtime
# and proper initrd etc. so this is good enough for now
../u-boot-apos/tools/mkimage -f arch/riscv/conf/apos.its fs/apos.itb
../u-boot-apos/tools/mkimage -A riscv -O apos -T script \
	-d arch/riscv/conf/boot.cmd fs/boot.scr

umount -l fs

qemu-system-riscv64 -machine virt \
	-kernel ../u-boot-apos/u-boot \
	-bios /usr/lib/riscv64-linux-gnu/opensbi/generic/fw_jump.elf \
	-device virtio-blk-device,drive=hd0 \
	-drive file=rootfs.img,format=raw,id=hd0 \
	-S -s -m 2G \
	-serial stdio
