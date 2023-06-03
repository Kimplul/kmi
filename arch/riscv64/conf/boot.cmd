fdt move ${fdt_addr} ${fdt_addr_r}
fdt addr ${fdt_addr_r}
load ${devtype} ${devnum} ${kernel_addr_r} kmi.itb
bootm ${kernel_addr_r} ${kernel_addr_r} ${fdt_addr_r}

# the above works for qemu, visionfive2 looks more like
#
# fdt move ${fdt_addr} ${fdt_addr_r}
# fdt addr ${fdt_addr_r}
# load virtio 0:1 0x80260000 kmi.bin
# load virtio 0:1 0x88300000 initrd
# fdt chosen 0x88300000 0x1000
# go 0x80260000 ${fdt_addr_r}
