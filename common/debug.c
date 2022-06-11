/**
 * @file debug.c
 * Handle printing to serial. Note that the serial drivers are only included
 * when running a debug build to save space in release mode.
 */

#include <apos/types.h>
#include <apos/debug.h>
#include <apos/bits.h>
#include <apos/vmem.h>
#include <apos/pmem.h>
#include <arch/vmem.h>
#include <libfdt.h>
#include <stdarg.h>

#if defined(DEBUG)

/** Debug context structure. */
struct dbg_info {
	/** Address of serial device in memory. */
	pm_t dbg_ptr;

	/** Type of serial device. */
	enum serial_dev dev;
};

/** Static debugging information. */
static struct dbg_info dbg_info = (struct dbg_info){ 0 };

/* forward declarations. */
static void __setup_dbg(pm_t pt, enum serial_dev dev);
static struct dbg_info __dbg_from_fdt(const void *fdt);

void init_dbg(const void *fdt)
{
	dbg_info = __dbg_from_fdt(fdt);
}

void setup_dmap_dbg()
{
	__setup_dbg(dbg_info.dbg_ptr, dbg_info.dev);
}

void setup_io_dbg(struct vmem *b)
{
	vm_t io_ptr = setup_kernel_io(b, dbg_info.dbg_ptr);
	__setup_dbg(io_ptr, dbg_info.dev);
}

/* if there arises a need for more supported serial drivers, I should probably
 * try to implement some kind of basic driver subsystem, but this is good enough
 * for now. */

/** NS16550A and compatible serial drivers. */
struct __packed ns16550a {
	/** Receiver buffer/transmitter holding register. */
	uint8_t data;

	/** Interrupt enable register. */
	uint8_t irq;

	/** Interrupt identity/FIFO control register. */
	uint8_t irq_id;

	/** Line control register. */
	uint8_t lcr;

	/** Modem control register. */
	uint8_t mcr;

	/** Line status register. */
	uint8_t lsr;

	/** Modem status register. */
	uint8_t msr;

	/** Scratch register. */
	uint8_t scr;
};

/** Line status data ready. */
#define LSR_DR (1 << 0)

/** Line status overrun error. */
#define LSR_OE (1 << 1)

/** Line status parity error. */
#define LSR_PE (1 << 2)

/** Line status framing error. */
#define LSR_FE (1 << 3)

/** Line status break interrupt. */
#define LSR_BI (1 << 4)

/** Line status transmitter holding register. */
#define LSR_THRE (1 << 5)

/** Line status transmitter empty. */
#define LSR_TEMT (1 << 6)

/** Line status error in RCVR FIFO. */
#define LSR_ERR (1 << 7)

/**
 * Address of ns16550a port. If other serial drivers are added, this should
 * maybe be made a void *.
 */
static struct ns16550a *port = 0;

/**
 * Serial transmitter empty.
 *
 * @return \c 0 if not empty, non-zero otherwise.
 */
static int __serial_tx_empty()
{
	return port->lsr & LSR_THRE;
}

/**
 * Put character out onto serial lines.
 *
 * @param c Character to output.
 */
static void __putchar(char c)
{
	if (!port)
		return;

	while (__serial_tx_empty() == 0)
		;

	port->data = c;
}

/**
 * Convert serial device name (from FDT) to serial device enumerator.
 *
 * @param dev_name Device name string.
 * @return Corresponding enumerator value.
 */
static enum serial_dev __serial_dev_enum(const char *dev_name)
{
	if (strncmp("ns16550", dev_name, 7) == 0)
		return NS16550A;

	return -1;
}

/**
 * Get debugging info from FDT.
 *
 * @param fdt Global FDT pointer.
 * @return Filled \ref dbg_info structure.
 */
static struct dbg_info __dbg_from_fdt(const void *fdt)
{
	int chosen_offset = fdt_path_offset(fdt, "/chosen");
	const char *stdout =
		fdt_getprop(fdt, chosen_offset, "stdout-path", NULL);

	int stdout_offset = fdt_path_offset(fdt, stdout);

	/* get serial device type */
	const char *dev_name = (const char *)fdt_getprop(fdt, stdout_offset,
	                                                 "compatible", NULL);

	enum serial_dev dev = __serial_dev_enum(dev_name);

	/* get serial device address */
	struct cell_info ci = get_reginfo(fdt, stdout);
	void *reg_ptr = (void *)fdt_getprop(fdt, stdout_offset, "reg", NULL);

	pm_t dbg_ptr = (pm_t)fdt_load_int_ptr(ci.addr_cells, reg_ptr);

	return (struct dbg_info){ dbg_ptr, dev };
}

/**
 * Set static port.
 *
 * @param pt Address of memory mapped serial device.
 * @param dev Chosen device.
 */
void __setup_dbg(vm_t pt, enum serial_dev dev)
{
	switch (dev) {
	case NS16550A:
		port = (struct ns16550a *)pt;
		break;
	}

	/* in the future possibly configure the serial connection, though the
	 * defaults (set by U-boot) seem to work alright */
}

/** Printf formatting left align flag. */
#define LEFT_FLAG (1 << 0)

/** Printf formatting explicit sign flag. */
#define SIGN_FLAG (1 << 1)

/** Printf formatting hash sign flag. */
#define HASH_FLAG (1 << 2)

/** Printf formatting zero padding flag. */
#define ZERO_FLAG (1 << 3)

/** Printf formatting ' flag. */
#define FMT_FLAG (1 << 4)

/** Printf formatting space flag. */
#define SPACE_FLAG (1 << 5)

/** Printf formatting long specifier flag. */
#define LONG_FLAG (1 << 6)

/** Printf formatting long long specifier flag. */
#define LLONG_FLAG (1 << 7)

/** Printf formatting short flag. */
#define SHORT_FLAG (1 << 8)

/** Printf formatting char flag. */
#define CHAR_FLAG (1 << 9)

/** Printf precision flag. */
#define PRECS_FLAG (1 << 11)

/** Printf unsigned flag. */
#define UNSIGN_FLAG (1 << 12)

/** Printf width flag. */
#define WIDTH_FLAG (1 << 13)

/** Printf padding flag. */
#define PAD_FLAG (1 << 14)

/** Printf continue flag. */
#define CONT 1

/** Printf stop flag. */
#define STOP 0

/**
 * Check if character is ASCII decimal digit.
 *
 * @param c Character to check.
 * @return \c true if character is ASCII decimal digit, \c false otherwise.
 */
static bool __is_digit(char c)
{
	return (c >= '0') && (c <= '9');
}

/**
 * Convert string to corresponding number (assuming int).
 *
 * @param s Number string.
 * @return Corresponding number.
 */
static int __atoi(const char *s)
{
	unsigned int i = 0;
	while (__is_digit(*s)) {
		i = i * 10 + (unsigned int)(*(s++) - '0');
	}

	return i;
}

/**
 * Calculate signed char from value using type interpretation.
 *
 * @param x Type to interpret value as.
 * @param value Value to interpret.
 * @param base Base to interpret value in.
 */
#define handle_type(x, value, base) \
	c = (x)value % (x)base;     \
	value = (x)value / (x)base;
/**
 * Convert number to string length.
 *
 * @param value Number to print.
 * @param base Base to print in.
 * @param flags Flags to output.
 * @param print Print number as well.
 * @return Length of corresponding string.
 */
static size_t __integral_val(ssize_t value, size_t base, size_t flags,
                             bool print)
{
	/* assume ascii numbers, which is why 'signed char' is probably fine */
	size_t ret = 0;
	signed char c = 0;


	if (!is_set(flags, UNSIGN_FLAG)) {
		/* signed values, only with i format */
		if (is_set(flags, LLONG_FLAG)) {
			handle_type(signed long long, value, base);
		} else if (is_set(flags, LONG_FLAG)) {
			handle_type(signed long, value, base);
		} else if (is_set(flags, SHORT_FLAG)) {
			handle_type(signed short, value, base);
		} else if (is_set(flags, CHAR_FLAG)) {
			handle_type(signed char, value, base);
		} else {
			handle_type(signed int, value, base);
		}

		/* convert negative results into actual characters */
		c = c < 0 ? -c : c;

	} else {
		/* unsigned values, everything else */
		if (is_set(flags, LLONG_FLAG)) {
			handle_type(unsigned long long, value, base);
		} else if (is_set(flags, LONG_FLAG)) {
			handle_type(unsigned long, value, base);
		} else if (is_set(flags, SHORT_FLAG)) {
			handle_type(unsigned short, value, base);
		} else if (is_set(flags, CHAR_FLAG)) {
			handle_type(unsigned char, value, base);
		} else {
			handle_type(unsigned int, value, base);
		}
	}

	if (base == 16)
		c += c > 9 ? 'a' - 10 : '0';
	else
		c += '0';

	if (value != 0)
		ret = __integral_val(value, base, flags, print);

	if (print)
		__putchar(c);

	return ret + 1;
}

/**
 * Print prefix corresponding to \c base.
 *
 * @param base Base to integer.
 * @return Length of prefix as string.
 */
static size_t __print_prefix(size_t base)
{
	size_t i = 0;

	const char *hex = "0x";
	const char *oct = "0";
	const char *bin = "0b";
	const char *empty = "";

	const char *prefix;

	if (base == 16)
		prefix = hex;
	else if (base == 8)
		prefix = oct;
	else if (base == 2)
		prefix = bin;
	else
		prefix = empty;

	for (; *prefix; ++i)
		__putchar(*prefix++);

	return i;
}

/**
 * Print padding.
 *
 * @param pad Number of characters to print.
 * @param pad_char Character to use as padding.
 * @return Number of characters printed.
 */
static size_t __print_padding(size_t pad, char pad_char)
{
	size_t i = 0;
	for (; i < pad; ++i) {
		__putchar(pad_char);
	}

	return i;
}

/**
 * Print signed value.
 *
 * @param value Value to print.
 * @param flags Flags to printing.
 * @return Number of characters written.
 */
static size_t __print_sign(ssize_t value, size_t flags)
{
	if (is_set(flags, LLONG_FLAG))
		value = (signed long long)value;
	else if (is_set(flags, LONG_FLAG))
		value = (signed long)value;
	else if (is_set(flags, SHORT_FLAG))
		value = (signed short)value;
	else if (is_set(flags, CHAR_FLAG))
		value = (signed char)value;
	else
		value = (signed int)value;

	if (value < 0) {
		__putchar('-');
		return 1;
	} else if (flags & SIGN_FLAG) {
		__putchar('+');
		return 1;
	}

	return 0;
}

/**
 * Length of integral value as string.
 *
 * @param value Value to convert to string.
 * @param base Base to interpret value as.
 * @param flags Formatting flags.
 * @return \see __integral_val().
 */
#define __integral_len(value, base, flags) __integral_val((value), (base), (flags), false)

/**
 * Print integral value as string.
 *
 * @param value Value to convert to string.
 * @param base Base to interpret value as.
 * @param flags Formatting flags.
 * @return \see __integral_val().
 */
#define __integral_print(value, base, flags) __integral_val((value), (base), (flags), true)

/**
 * Print integral value.
 *
 * @param value Value to print.
 * @param base Base to print value in.
 * @param flags Flags to printing.
 * @param width Minimum width of printing.
 * @return Number of characters written.
 */
static size_t __print_integral(ssize_t value, size_t base, size_t flags,
                               size_t width)
{
	size_t ret = 0;
	size_t raw_len = __integral_len(value, base, flags);
	ssize_t pad = is_set(flags, PAD_FLAG) ? width - raw_len : 0;

	/* depending on which flags are set, the prefix, sign and right justify has to
	 * be ordereder differently. */
	if (is_set(flags, ZERO_FLAG)) {
		if (!is_set(flags, UNSIGN_FLAG))
			ret += __print_sign(value, flags);

		if (is_set(flags, HASH_FLAG))
			ret += __print_prefix(base);

		if (pad > 0 && !is_set(flags, LEFT_FLAG))
			ret += __print_padding(pad, '0');

	} else if (is_set(flags, SPACE_FLAG)) {
		if (pad > 0 && !is_set(flags, LEFT_FLAG))
			ret += __print_padding(pad, ' ');

		if (!is_set(flags, UNSIGN_FLAG))
			ret += __print_sign(value, flags);

		if (is_set(flags, HASH_FLAG))
			ret += __print_prefix(base);
	} else {
		if (!is_set(flags, UNSIGN_FLAG))
			ret += __print_sign(value, flags);

		if (is_set(flags, HASH_FLAG))
			ret += __print_prefix(base);
	}

	/* print value itself */
	ret += __integral_print(value, base, flags);

	/* left-justify */
	if (is_set(flags, ZERO_FLAG)) {
		if (pad > 0 && is_set(flags, LEFT_FLAG))
			ret += __print_padding(pad, '0');
	} else if (is_set(flags, SPACE_FLAG)) {
		if (pad > 0 && is_set(flags, LEFT_FLAG))
			ret += __print_padding(pad, ' ');
	}

	return ret;
}

void dbg(const char *fmt, ...)
{
	/* largely inspired by
	 * https://github.com/mpaland/printf/blob/master/printf.c
	 */

	/* Note that X is binary formatting, because who uses uppercase hex? */

	va_list vl;
	va_start(vl, fmt);

	size_t chars_written = 0;

	while (*fmt) {
		if (*fmt != '%') {
			__putchar(*fmt++);
			chars_written++;
			continue;
		}

		fmt++;
		if (*fmt == '%') {
			/* literal percent sign */
			__putchar('%');
			chars_written++;
			fmt++;
			continue;
		}

		/* check flags */
		size_t flags = 0;
		int a = STOP;
		do {
			switch (*fmt) {
			case ' ':
				set_bit(flags, SPACE_FLAG);
				fmt++;
				a = CONT;
				break;

			case '-':
				set_bit(flags, LEFT_FLAG);
				fmt++;
				a = CONT;
				break;

			case '+':
				set_bit(flags, SIGN_FLAG);
				fmt++;
				a = CONT;
				break;

			case '#':
				set_bit(flags, HASH_FLAG);
				fmt++;
				a = CONT;
				break;

			case '0':
				set_bit(flags, ZERO_FLAG);
				fmt++;
				a = CONT;
				break;

			case '\'':
				set_bit(flags, FMT_FLAG);
				fmt++;
				a = CONT;
				break;

			default:
				a = STOP;
				break;
			}
		} while (a != STOP);

		/* check width */
		size_t width = 0;
		if (__is_digit(*fmt)) {
			width = __atoi(fmt++);
			set_bit(flags, WIDTH_FLAG | PAD_FLAG | SPACE_FLAG);
		} else if (*fmt == '*') {
			int w = va_arg(vl, int);
			if (w < 0) {
				width = -w;
				set_bit(flags, LEFT_FLAG);
			} else {
				width = w;
			}
			set_bit(flags, WIDTH_FLAG | PAD_FLAG | SPACE_FLAG);
			fmt++;
		}

		/* check precision */
		size_t precision = 0;
		if (*fmt == '.') {
			fmt++;
			set_bit(flags, PRECS_FLAG | PAD_FLAG | ZERO_FLAG);
			if (__is_digit(*fmt)) {
				precision = __atoi(fmt++);
			} else if (*fmt == '*') {
				precision = va_arg(vl, int);
				fmt++;
			}
		}

		/* check length */
		switch (*fmt) {
		case 'l':
			fmt++;
			if (*fmt == 'l') {
				set_bit(flags, LLONG_FLAG);
				fmt++;
			} else {
				set_bit(flags, LONG_FLAG);
			}
			break;

		case 'h':
			fmt++;
			if (*fmt == 'h') {
				set_bit(flags, CHAR_FLAG);
				fmt++;
			} else {
				set_bit(flags, SHORT_FLAG);
			}
			break;

		case 'j':
			fmt++;
			if (sizeof(intmax_t) == sizeof(long))
				set_bit(flags, LONG_FLAG);
			else
				set_bit(flags, LLONG_FLAG);
			break;

		case 'z':
			fmt++;
			if (sizeof(size_t) == sizeof(long))
				set_bit(flags, LONG_FLAG);
			else
				set_bit(flags, LLONG_FLAG);
			break;

		case 't':
			fmt++;
			if (sizeof(ptrdiff_t) == sizeof(long))
				set_bit(flags, LONG_FLAG);
			else
				set_bit(flags, LLONG_FLAG);
			break;
		}

		/* read actual specifier */
		size_t base = 10;
		size_t value = 0;
		int i = -1;
		const char *s = 0;
		void *p = 0;
		int *n = 0;
		char c = 0;

		switch (*fmt) {
		case 'd':
		case 'i':
		case 'u':
		case 'x':
		case 'X':
		case 'o':
		case 'b':
			/* integer handling */
			switch (*fmt) {
			case 'x':
				base = 16;
				break;
			case 'X':
				base = 2;
				break;
			case 'o':
				base = 8;
				break;
			default:
				base = 10;
				break;
			}

			if (base == 10)
				clear_bit(flags, HASH_FLAG);

			/* precision takes precedence */
			if (is_set(flags, PRECS_FLAG))
				width = precision;

			/* formatting doesn't apply to decimal integers
			 * */
			if (*fmt != 'i' && *fmt != 'd') {
				clear_bit(flags, SIGN_FLAG);
				set_bit(flags, UNSIGN_FLAG);
			}

			if (is_set(flags, LLONG_FLAG))
				value = va_arg(vl, long long);
			else if (is_set(flags, LONG_FLAG))
				value = va_arg(vl, long);
			else
				value = va_arg(vl, int);

			chars_written +=
				__print_integral(value, base, flags, width);
			fmt++;
			break;

		case 'c':
			c = va_arg(vl, int);
			__putchar(c);
			chars_written++;
			fmt++;
			break;

		case 's':
			s = va_arg(vl, const char *);

			if (is_set(flags, PRECS_FLAG))
				i = precision;

			for (; *s && i--;) {
				__putchar(*s++);
				chars_written++;
			}
			fmt++;
			break;

		case 'p':
			p = va_arg(vl, void *);
			set_bit(flags, UNSIGN_FLAG | HASH_FLAG);

			if (sizeof(void *) == sizeof(long))
				set_bit(flags, LONG_FLAG);
			else
				set_bit(flags, LLONG_FLAG);

			chars_written +=
				__print_integral((ssize_t)p, 16, flags, width);
			fmt++;
			break;

		case 'n':
			n = va_arg(vl, int *);
			*n = chars_written;
			fmt++;
			break;
		}
	}

	va_end(vl);
}

#endif /* DEBUG */
