/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

.section .init.start
.global _start
/* Entry point to the kernel loader. */
_start:
/* get load address */
auipc a1, 0
/* keep using bootloader stack for now */
//li sp, PM_STACK_TOP
/* make sure there's no garbage in tp, important for id assignment */
li tp, 0
call init

.section .text
.global jump_to_kernel
jump_to_kernel:
li sp, VM_STACK_TOP // load virtual stack address
li fp, 0
li gp, 0
jr a2 // jump to kernel

/* there should be a kernel payload after this address */
.section .top
.global __kernel
__kernel:
