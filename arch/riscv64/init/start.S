/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/* generic u-boot passes argc and argv as if we were a regular
 * program, so our load address must be placed third.
 * If we have a 'custom' u-boot with kmi support, the fdt
 * will be passed to us directly, so the load address can go second.
 */
#if GENERIC_UBOOT
# define LOAD_REG a2
# define INIT init_go
#else
# define LOAD_REG a1
# define INIT init
#endif

.section .init
.global _start
/* Entry point to the kernel loader. */
_start:
/* get load address */
auipc LOAD_REG, 0
/* keep using bootloader stack for now */
//li sp, PM_STACK_TOP
/* make sure there's no garbage in tp, important for id assignment */
li tp, 0
call INIT
// INIT shouldn't return, but if we do we might as well just jump back to u-boot
// or wherever
ret

.section .text
.global jump_to_kernel
jump_to_kernel:
li sp, VM_STACK_TOP // load virtual stack address
li fp, 0
li gp, 0
jr a2 // jump to kernel
