/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

.text
.global _start
_start:
li a0, 0 /* 0 should be NOOP */
li a1, 1
li a2, 2
li a3, 3
li a4, 4
ecall
