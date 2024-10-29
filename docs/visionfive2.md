# StarFive VisionFive2

## Booting

Ideally I would fork StarFive's u-boot fork and add 'native' support for my
kernel, but it turns out that their fork is at the time of writing borked and
I wasn't able to get anything to boot with it. As such, we'll use their prebuilt
bootloader and boot in a more generic way.

The following is how to boot with an SD card. The board seems to support
serial and ethernet booting, but I haven't looked into how they work.

1. Build `kmi` with `GENERIC_UBOOT=1`.

2. Create three partitions on an SD card of your choice:
```
sudo sgdisk \
	     --new=1:4096:8191 --change-name=1:"spl" --typecode=1:2E54B353-1271-4842-806F-E436D6AF6985 \
	     --new=2:8192:16383 --change-name=2:"uboot" --typecode=2:5B193300-FC78-40CD-8002-E86C45580B47 \
	     --new=3:16384:0 --change-name=3:"data" \
	     /dev/sdX
```

3. Prepare partitions with data:
```
sudo dd if=u-boot-spl.bin.normal.out of=/dev/sdX1
sudo dd if=visionfive2_fw_payload.img of=/dev/sdX2
sudo mkfs.ext2 /dev/sdX3
```

4. Copy `kmi.bin` and `initrd` to SD card:
```
sudo mount /dev/sdX3 /mnt/sdcard # or wherever
sudo cp kmi.bin arch/riscv64/conf/initrd /mnt/sdcard
sudo umount /mnt/sdcar
sync
```

With the SD card ready to go, reset the device and manually run these commands:

1. `fdt move ${fdtaddr} ${fdt_addr_r}`
2. `load mmc 1:3 0x47000000 kmi.bin`
3. `load mmc 1:3 0x48000000 initrd`
4. `fdt chosen 0x48000000 0x48001200`
5. `go 0x47000000 ${fdt_addr_r}`

These commands are pretty hard coded and might not work in the future. At least
currently `${fdt_addr_r}` is `0x46000000`, so you can probably see where the
other addresses come from. Also, the `/chosen` node has to reflect the size of
the `initrd`. Ideally I would like `u-boot` to figure it out itself, but haven't
managed to do so at least yet.

Other things to note:

U-boot likes to skip the `0x` prefix for
hexadecimal values, which can confuse my string converter.
Check that the strings you pass to `go` have their correct prefixes.

Depending on the u-boot version, the `fdt chosen` command either takes
the end address or the size of the `initrd`.

It should be possible to add the above commands to a script to run
automatically, though I haven't personally done this yet.
