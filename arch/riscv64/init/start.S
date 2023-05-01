/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

.section .init.start
.global _start
/* Entry point to the kernel loader. */
_start:
/* load static stack */
li sp, PM_STACK_TOP
/* make sure there's no garbage in tp, important for id assignment */
li tp, 0
call init

.section .text
.global jump_to_kernel
jump_to_kernel:
mv a2, a0 // store kernel addr
mv a0, a1 // move fdt pointer to first argument
jr a2 // jump to kernel
