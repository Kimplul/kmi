/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2023, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_PANIC_H
#define KMI_PANIC_H

#include <kmi/attrs.h>

/**
 * @file panic.h
 * Kernel panic handler.
 */

/**
 * Call on kernel panic, tries to reboot the system. Failing that, spin.
 * Ideally registers and kernel state would be printed, but keep things simple
 * for now.
 *
 * @param pc Address where fault occured. Should preferably be in the kernel.
 * @param addr Possibly associated address.
 * @param cause Possible error code associated with panic. Page fault, etc.
 */
__noreturn void kernel_panic(void *pc, void *addr, long cause);

/**
 * Call on some other, like userspace doing something that we currently can't
 * deal with. Tries to reboot the system. Failing that, spin.
 * Ideally registers and kernel state would be printed, but keep things simple
 * for now.
 *
 * @param pc Address where fault occured. Should preferably be in the kernel.
 * @param addr Possibly associated address.
 * @param cause Possible error code associated with panic. Page fault, etc.
 */
__noreturn void unhandled_panic(void *pc, void *addr, long cause);

#endif /* KMI_PANIC_H */
