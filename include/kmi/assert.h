/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_ASSERT_H
#define KMI_ASSERT_H

/**
 * @file assert.h
 * Assertions. Note that contrary to how assertios usually function, kmi has
 * three different levels of assertions: Catastrophic, hard and soft.
 *
 * Soft assertions merely warn about something that might cause issues, but let
 * the execution continue normally.
 *
 * Hard assertions warn about the assertion not holding and returns from the
 * function.
 *
 * Catastrophic assertions warn about the assertion and crash the kernel.
 */

#include <kmi/debug.h>
#include <kmi/utils.h>

/** \todo should this exit or do something explosive like that? */
#if defined(ASSERT)

/**
 * The kernel is in an irrepairable state, just give up.
 * Should maybe call kernel_panic()?
 *
 * @param x Condition to check for.
 */
#define assert(x)                                            \
	do {                                                 \
		if (unlikely(!(x))) {                        \
			error("assertion failed: " #x "\n"); \
			while (1) {                          \
			}                                    \
		}                                            \
	} while (0);

#else
#define assert(x)
#endif

#endif /* KMI_ASSERT_H */
