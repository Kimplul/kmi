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

/* "print" hello world */
li a0, 1
li a1, 'H'
ecall
li a0, 1
li a1, 'e'
ecall
li a0, 1
li a1, 'l'
ecall
li a0, 1
li a1, 'l'
ecall
li a0, 1
li a1, 'o'
ecall
li a0, 1
li a1, ','
ecall
li a0, 1
li a1, ' '
ecall
li a0, 1
li a1, 'w'
ecall
li a0, 1
li a1, 'o'
ecall
li a0, 1
li a1, 'r'
ecall
li a0, 1
li a1, 'l'
ecall
li a0, 1
li a1, 'd'
ecall
li a0, 1
li a1, '!'
ecall
li a0, 1
li a1, '\n'
ecall
