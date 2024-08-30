#include <stdarg.h>
#include <kmi/types.h>

#include "printf.h"
#include "sys.h"

/* Directly copied from src/debug.c, which was copied from
 * https://github.com/mpaland/printf/blob/master/printf.c
 */

/** Ask kernel to print out one character for us. Only works if kernel was
 * compiled with DEBUG=1. */
static void __putchar(char c)
{
	sys_putch(c);
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


	if (flags & UNSIGN_FLAG) {
		/* signed values, only with i format */
		if (flags & LLONG_FLAG) {
			handle_type(signed long long, value, base);
		} else if (flags & LONG_FLAG) {
			handle_type(signed long, value, base);
		} else if (flags & SHORT_FLAG) {
			handle_type(signed short, value, base);
		} else if (flags & CHAR_FLAG) {
			handle_type(signed char, value, base);
		} else {
			handle_type(signed int, value, base);
		}

		/* convert negative results into actual characters */
		c = c < 0 ? -c : c;

	} else {
		/* unsigned values, everything else */
		if (flags & LLONG_FLAG) {
			handle_type(unsigned long long, value, base);
		} else if (flags & LONG_FLAG) {
			handle_type(unsigned long, value, base);
		} else if (flags & SHORT_FLAG) {
			handle_type(unsigned short, value, base);
		} else if (flags & CHAR_FLAG) {
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
 * Print out a string.
 *
 * @param s String to print out.
 * @return Bytes printed.
 */
static size_t __puts(const char *s)
{
	size_t i = 0;
	while (s[i]) {
		__putchar(s[i++]);
	}

	return i;
}

/**
 * Print prefix corresponding to \c base.
 *
 * @param base Base to integer.
 * @return Length of prefix as string.
 */
static size_t __print_prefix(size_t base)
{
	switch (base) {
	case 16: return __puts("0x");
	case 8: return __puts("0");
	case 2: return __puts("0b");
	default: break;
	}

	return 0;
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
	if (flags & LLONG_FLAG)
		value = (signed long long)value;
	else if (flags & LONG_FLAG)
		value = (signed long)value;
	else if (flags & SHORT_FLAG)
		value = (signed short)value;
	else if (flags & CHAR_FLAG)
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
#define __integral_len(value, base, flags) __integral_val((value), (base), \
							  (flags), false)

/**
 * Print integral value as string.
 *
 * @param value Value to convert to string.
 * @param base Base to interpret value as.
 * @param flags Formatting flags.
 * @return \see __integral_val().
 */
#define __integral_print(value, base, flags) __integral_val((value), (base), \
							    (flags), true)

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
	ssize_t pad = (flags & PAD_FLAG) ? width - raw_len : 0;

	/* depending on which flags are set, the prefix, sign and right justify has to
	 * be ordereder differently. */
	if (flags & ZERO_FLAG) {
		if (!(flags & UNSIGN_FLAG))
			ret += __print_sign(value, flags);

		if (flags & HASH_FLAG)
			ret += __print_prefix(base);

		if (pad > 0 && !(flags & LEFT_FLAG))
			ret += __print_padding(pad, '0');

	} else if (flags & SPACE_FLAG) {
		if (pad > 0 && !(flags & LEFT_FLAG))
			ret += __print_padding(pad, ' ');

		if (flags & UNSIGN_FLAG)
			ret += __print_sign(value, flags);

		if (flags & HASH_FLAG)
			ret += __print_prefix(base);
	} else {
		if (!(flags & UNSIGN_FLAG))
			ret += __print_sign(value, flags);

		if (flags & HASH_FLAG)
			ret += __print_prefix(base);
	}

	/* print value itself */
	ret += __integral_print(value, base, flags);

	/* left-justify */
	if (flags & ZERO_FLAG) {
		if (pad > 0 && (flags & LEFT_FLAG))
			ret += __print_padding(pad, '0');
	} else if (flags & SPACE_FLAG) {
		if (pad > 0 && (flags & LEFT_FLAG))
			ret += __print_padding(pad, ' ');
	}

	return ret;
}

int printf(const char *fmt, ...)
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
				flags |= SPACE_FLAG;
				fmt++;
				a = CONT;
				break;

			case '-':
				flags |= LEFT_FLAG;
				fmt++;
				a = CONT;
				break;

			case '+':
				flags |= SIGN_FLAG;
				fmt++;
				a = CONT;
				break;

			case '#':
				flags |= HASH_FLAG;
				fmt++;
				a = CONT;
				break;

			case '0':
				flags |= ZERO_FLAG;
				fmt++;
				a = CONT;
				break;

			case '\'':
				flags |= FMT_FLAG;
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
			flags |= WIDTH_FLAG | PAD_FLAG | SPACE_FLAG;
		} else if (*fmt == '*') {
			int w = va_arg(vl, int);
			if (w < 0) {
				width = -w;
				flags |= LEFT_FLAG;
			} else {
				width = w;
			}
			flags |= WIDTH_FLAG | PAD_FLAG | SPACE_FLAG;
			fmt++;
		}

		/* check precision */
		size_t precision = 0;
		if (*fmt == '.') {
			fmt++;
			flags |= PRECS_FLAG | PAD_FLAG | ZERO_FLAG;
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
				flags |= LLONG_FLAG;
				fmt++;
			} else {
				flags |= LONG_FLAG;
			}
			break;

		case 'h':
			fmt++;
			if (*fmt == 'h') {
				flags |= CHAR_FLAG;
				fmt++;
			} else {
				flags |= SHORT_FLAG;
			}
			break;

		case 'j':
			fmt++;
			if (sizeof(intmax_t) == sizeof(long))
				flags |= LONG_FLAG;
			else
				flags |= LLONG_FLAG;
			break;

		case 'z':
			fmt++;
			if (sizeof(size_t) == sizeof(long))
				flags |= LONG_FLAG;
			else
				flags |= LLONG_FLAG;
			break;

		case 't':
			fmt++;
			if (sizeof(ptrdiff_t) == sizeof(long))
				flags |= LONG_FLAG;
			else
				flags |= LLONG_FLAG;
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
				flags &= ~HASH_FLAG;

			/* precision takes precedence */
			if (flags & PRECS_FLAG)
				width = precision;

			/* formatting doesn't apply to decimal integers
			 * */
			if (*fmt != 'i' && *fmt != 'd') {
				flags &= ~SIGN_FLAG;
				flags |= UNSIGN_FLAG;
			}

			if (flags & LLONG_FLAG)
				value = va_arg(vl, long long);
			else if (flags & LONG_FLAG)
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

			if (flags & PRECS_FLAG)
				i = precision;

			for (; *s && i--;) {
				__putchar(*s++);
				chars_written++;
			}
			fmt++;
			break;

		case 'p':
			p = va_arg(vl, void *);
			flags |= UNSIGN_FLAG | HASH_FLAG;

			if (sizeof(void *) == sizeof(long))
				flags |= LONG_FLAG;
			else
				flags |= LLONG_FLAG;

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
	return chars_written;
}
