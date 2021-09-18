fdt move ${fdt_addr} ${fdt_addr_r}
fdt addr ${fdt_addr_r}
load ${devtype} ${devnum} ${kernel_addr_r} apos.itb
bootm ${kernel_addr_r} ${kernel_addr_r} ${fdt_addr_r}
