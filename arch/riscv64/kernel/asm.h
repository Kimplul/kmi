/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

/**
 * @file asm.h
 * Stuff to make assembly easier to write on both riscv64 and riscv32.
 */

#if __riscv_xlen == 64

/** Store register. */
#define sr sd

/** Load register. */
#define lr ld

/** Shift count to get a register sized value. */
#define RW_SHIFT 3

#else

/** Store register. */
#define sr sw

/** Load register. */
#define lr lw

/** Shift count to get register sized value. */
#define RW_SHIFT 2
#endif
