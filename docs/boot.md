I need to figure out how to write MarkDown files, but the general procedure of a
boot is that we

1) Assume we start of in real memory mode, i.e. no MMU active.

2) arch/whatever/init/init.c kickstarts us into virtual memory with a direct
mapping, with offset VM_DMAP aliasing to physical address 0.

3) The kernel proper is moved to its correct location in this direct mapping.

4) The kernel proper is jumped to, after which the kernel does whatever.
