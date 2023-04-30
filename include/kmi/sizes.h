/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_SIZES_H
#define KMI_SIZES_H

/**
 * @file sizes.h
 * Shorthands for some power-of-two sizes.
 */

#if !defined(__ASSEMBLER__)

/* value format used by the compiler. */

/** 1 byte. */
#define SZ_1 0x000000000001UL

/** 2 bytes. */
#define SZ_2 0x000000000002UL

/** 4 bytes. */
#define SZ_4 0x000000000004UL

/** 8 bytes. */
#define SZ_8 0x000000000008UL

/** 16 bytes. */
#define SZ_16 0x000000000010UL

/** 32 bytes. */
#define SZ_32 0x000000000020UL

/** 64 bytes. */
#define SZ_64 0x000000000040UL

/** 128 bytes. */
#define SZ_128 0x000000000080UL

/** 256 bytes. */
#define SZ_256 0x000000000100UL

/** 512 bytes. */
#define SZ_512 0x000000000200UL

/** 1KiB. */
#define SZ_1K 0x000000000400UL

/** 2KiB. */
#define SZ_2K 0x000000000800UL

/** 4KiB. */
#define SZ_4K 0x000000001000UL

/** 8KiB. */
#define SZ_8K 0x000000002000UL

/** 16KiB. */
#define SZ_16K 0x000000004000UL

/** 32KiB. */
#define SZ_32K 0x000000008000UL

/** 64KiB. */
#define SZ_64K 0x000000010000UL

/** 128KiB. */
#define SZ_128K 0x000000020000UL

/** 256KiB. */
#define SZ_256K 0x000000040000UL

/** 512KiB. */
#define SZ_512K 0x000000080000UL

/** 1MiB. */
#define SZ_1M 0x000000100000UL

/** 2MiB. */
#define SZ_2M 0x000000200000UL

/** 4MiB. */
#define SZ_4M 0x000000400000UL

/** 8MiB. */
#define SZ_8M 0x000000800000UL

/** 16MiB. */
#define SZ_16M 0x000001000000UL

/** 32MiB. */
#define SZ_32M 0x000002000000UL

/** 64MiB. */
#define SZ_64M 0x000004000000UL

/** 128MiB. */
#define SZ_128M 0x000008000000UL

/** 256MiB. */
#define SZ_256M 0x000010000000UL

/** 512MiB. */
#define SZ_512M 0x000020000000UL

/** 1GiB. */
#define SZ_1G 0x000040000000UL

/** 2GiB. */
#define SZ_2G 0x000080000000UL

/** 4GiB. */
#define SZ_4G 0x000100000000UL

/** 8GiB. */
#define SZ_8G 0x000200000000UL

/** 16GiB. */
#define SZ_16G 0x000400000000UL

/** 32GiB. */
#define SZ_32G 0x000800000000UL

/** 64GiB. */
#define SZ_64G 0x001000000000UL

/** 128GiB. */
#define SZ_128G 0x002000000000UL

/** 256GiB. */
#define SZ_256G 0x004000000000UL

/** 512GiB. */
#define SZ_512G 0x008000000000UL

/** 1TiB. */
#define SZ_1T 0x010000000000UL

/** 2TiB. */
#define SZ_2T 0x020000000000UL

/** 4TiB. */
#define SZ_4T 0x040000000000UL

/** 8TiB. */
#define SZ_8T 0x080000000000UL

/** 16TiB. */
#define SZ_16T 0x100000000000UL

/** 32TiB. */
#define SZ_32T 0x200000000000UL

/** 64TiB. */
#define SZ_64T 0x400000000000UL

/** 128TiB. */
#define SZ_128T 0x8000000000000UL

#else

/* value format used by the assembler. */
#define SZ_1 0x000000000001
#define SZ_2 0x000000000002
#define SZ_4 0x000000000004
#define SZ_8 0x000000000008
#define SZ_16 0x000000000010
#define SZ_32 0x000000000020
#define SZ_64 0x000000000040
#define SZ_128 0x000000000080
#define SZ_256 0x000000000100
#define SZ_512 0x000000000200

#define SZ_1K 0x000000000400
#define SZ_2K 0x000000000800
#define SZ_4K 0x000000001000
#define SZ_8K 0x000000002000
#define SZ_16K 0x000000004000
#define SZ_32K 0x000000008000
#define SZ_64K 0x000000010000
#define SZ_128K 0x000000020000
#define SZ_256K 0x000000040000
#define SZ_512K 0x000000080000

#define SZ_1M 0x000000100000
#define SZ_2M 0x000000200000
#define SZ_4M 0x000000400000
#define SZ_8M 0x000000800000
#define SZ_16M 0x000001000000
#define SZ_32M 0x000002000000
#define SZ_64M 0x000004000000
#define SZ_128M 0x000008000000
#define SZ_256M 0x000010000000
#define SZ_512M 0x000020000000

#define SZ_1G 0x000040000000
#define SZ_2G 0x000080000000
#define SZ_4G 0x000100000000
#define SZ_8G 0x000200000000
#define SZ_16G 0x000400000000
#define SZ_32G 0x000800000000
#define SZ_64G 0x000400000000
#define SZ_128G 0x000800000000
#define SZ_256G 0x001000000000
#define SZ_512G 0x002000000000

#define SZ_1T 0x004000000000
#define SZ_2T 0x008000000000
#define SZ_4T 0x010000000000
#define SZ_8T 0x020000000000
#define SZ_16T 0x040000000000
#define SZ_32T 0x080000000000
#define SZ_64T 0x100000000000
#define SZ_128T 0x200000000000
#define SZ_256T 0x400000000000
#define SZ_512T 0x800000000000

#endif

#endif /* KMI_SIZES_H */
