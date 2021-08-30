#include <apos/types.h>
#include <apos/debug.h>
#include <apos/bits.h>
#include <stdarg.h>

#ifdef DEBUG

/* if there arises a need for more supported serial drivers, I should probably
 * try to implement some kind of basic driver subsystem, but this is good enough
 * for now. */

struct __packed ns16550a {
	union {
		struct {
			uint8_t data;
			uint8_t irq;
		};

		struct {
			uint8_t lsbr;
			uint8_t msbr;
		};
	};

	uint8_t irq_id;
	uint8_t lcr;
	uint8_t mcr;
	uint8_t lsr;
	uint8_t msr;
	uint8_t scr;
};

#define LSR_DR   (1 << 0)
#define LSR_OE   (1 << 1)
#define LSR_PE   (1 << 2)
#define LSR_FE   (1 << 3)
#define LSR_BI   (1 << 4)
#define LSR_THRE (1 << 5)
#define LSR_TEMT (1 << 6)
#define LSR_ERR  (1 << 7)

static struct ns16550a *port = 0;

static int __serial_tx_empty()
{
	return port->lsr & LSR_THRE;
}

static void __putchar(char c)
{
	if (!port)
		return;

	while(__serial_tx_empty() == 0);

	port->data = c;
}

void dbg_init(void *pt, enum serial_dev_t dev)
{
	switch (dev) {
		case NS16550A:
			port = pt;
			break;
	}

	/* in the future possibly configure the serial connection, though the
	 * defaults (set by U-boot) seem to work alright */
}

#define LEFT_FLAG  (1 << 0)
#define SIGN_FLAG  (1 << 1)
#define HASH_FLAG  (1 << 2)
#define ZERO_FLAG  (1 << 3)
#define FMT_FLAG   (1 << 4)
#define SPACE_FLAG (1 << 5)
#define LONG_FLAG  (1 << 6)
#define LLONG_FLAG (1 << 7)
#define SHORT_FLAG (1 << 8)
#define CHAR_FLAG  (1 << 9)
#define PRECS_FLAG (1 << 11)
#define UNSIGN_FLAG (1 << 12)
#define WIDTH_FLAG  (1 << 13)
#define PAD_FLAG (1 << 14)

#define CONT 1
#define STOP 0

static bool __is_digit(char c)
{
	return (c >= '0') && (c <= '9');
}

static int __atoi(const char *s)
{
	unsigned int i = 0;
	while(__is_digit(*s)){
		i = i * 10 + (unsigned int)(*(s++) - '0');
	}

	return i;
}

static size_t __integral_val(ssize_t value, size_t base, size_t flags, bool print)
{
	/* assume ascii numbers, which is why 'signed char' is probably fine */
	size_t ret = 0;
	signed char c = 0;

#define handle_type(x)\
	c = (x)value % (x)base;\
	value = (x)value / (x)base;

	if(!__is_set(flags, UNSIGN_FLAG)){
		/* signed values, only with i format */
		if(__is_set(flags, LLONG_FLAG)){
			handle_type(signed long long);
		}
		else if(__is_set(flags, LONG_FLAG)){
			handle_type(signed long);
		}
		else if(__is_set(flags, SHORT_FLAG)){
			handle_type(signed short);
		}
		else if(__is_set(flags, CHAR_FLAG)){
			handle_type(signed char);
		}
		else {
			handle_type(signed int);
		}

		/* convert negative results into actual characters */
		c = c < 0 ? -c : c;

	} else {
		/* unsigned values, everything else */
		if(__is_set(flags, LLONG_FLAG)){
			handle_type(unsigned long long);
		}
		else if(__is_set(flags, LONG_FLAG)){
			handle_type(unsigned long);
		}
		else if(__is_set(flags, SHORT_FLAG)){
			handle_type(unsigned short);
		}
		else if(__is_set(flags, CHAR_FLAG)){
			handle_type(unsigned char);
		}
		else {
			handle_type(unsigned int);
		}
	}

#undef handle_type

	if(base == 16)
		c += c >= 9 ? 'a' - 10 : '0';
	else
		c += '0';

	if(value != 0)
		ret = __integral_val(value, base, flags, print);

	if(print)
		__putchar(c);

	return ret + 1;
}

static size_t __print_prefix(size_t base)
{
	size_t i = 0;

	const char *hex = "0x";
	const char *oct = "0";
	const char *bin = "0b";

	const char *prefix;

	if(base == 16)
		prefix = hex;
	else if(base == 8)
		prefix = oct;
	else if(base == 2)
		prefix = bin;

	for(; *prefix ; ++i)
		__putchar(*prefix++);

	return i;
}

static size_t __print_padding(size_t pad, char pad_char)
{
	size_t i = 0;
	for(; i < pad; ++i){
		__putchar(pad_char);
	}

	return i;
}

static size_t __print_sign(ssize_t value, size_t flags)
{
	if(__is_set(flags, LLONG_FLAG))
		value = (signed long long)value;
	else if(__is_set(flags, LONG_FLAG))
		value = (signed long)value;
	else if(__is_set(flags, SHORT_FLAG))
		value = (signed short)value;
	else if(__is_set(flags, CHAR_FLAG))
		value = (signed char)value;
	else
		value = (signed int)value;

	if(value < 0){
		__putchar('-');
		return 1;
	}
	else if(flags & SIGN_FLAG) {
		__putchar('+');
		return 1;
	}

	return 0;
}

static size_t __print_integral(ssize_t value, size_t base,
		size_t flags, size_t width)
{
#define __integral_len(a, b, c) __integral_val((a), (b), (c), false)
#define __integral_print(a, b, c) __integral_val((a), (b), (c), true)

	size_t ret = 0;
	size_t raw_len = __integral_len(value, base, flags);
	ssize_t pad = __is_set(flags, PAD_FLAG) ? width - raw_len : 0;

	/* depending on which flags are set, the prefix, sign and right justify has to
	 * be ordereder differently. */
	if(__is_set(flags, ZERO_FLAG)){
		if(!__is_set(flags, UNSIGN_FLAG))
			ret += __print_sign(value, flags);

		if(__is_set(flags, HASH_FLAG))
			ret += __print_prefix(base);

		if(pad > 0 && !__is_set(flags, LEFT_FLAG))
			ret += __print_padding(pad, '0');

	} else if (__is_set(flags, SPACE_FLAG)){
		if(pad > 0 && !__is_set(flags, LEFT_FLAG))
			ret += __print_padding(pad, ' ');

		if(!__is_set(flags, UNSIGN_FLAG))
			ret += __print_sign(value, flags);

		if(__is_set(flags, HASH_FLAG))
			ret += __print_prefix(base);
	} else {
		if(!__is_set(flags, UNSIGN_FLAG))
			ret += __print_sign(value, flags);

		if(__is_set(flags, HASH_FLAG))
			ret += __print_prefix(base);
	}

	/* print value itself */
	ret += __integral_print(value, base, flags);

	/* left-justify */
	if(__is_set(flags, ZERO_FLAG)){
		if(pad > 0 && __is_set(flags, LEFT_FLAG))
			ret += __print_padding(pad, '0');
	} else if(__is_set(flags, SPACE_FLAG)) {
		if(pad > 0 && __is_set(flags, LEFT_FLAG))
			ret += __print_padding(pad, ' ');
	}

	return ret;

#undef __integral_len
#undef __integral_print
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

	while(*fmt){

		if (*fmt != '%'){
			__putchar(*fmt++);
			chars_written++;
			continue;
		}

		fmt++;
		if(*fmt == '%'){
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
			switch(*fmt){
				case ' ':
					__set_bit(flags, SPACE_FLAG); fmt++; a = CONT;
					break;

				case '-':
					__set_bit(flags, LEFT_FLAG); fmt++; a = CONT;
					break;

				case '+':
					__set_bit(flags, SIGN_FLAG); fmt++; a = CONT;
					break;

				case '#':
					__set_bit(flags, HASH_FLAG); fmt++; a = CONT;
					break;

				case '0': __set_bit(flags, ZERO_FLAG); fmt++; a = CONT;
					  break;

				case '\'':
					  __set_bit(flags, FMT_FLAG); fmt++; a = CONT;
					  break;

				default:
					  a = STOP;
					  break;
			}
		} while (a != STOP);

		/* check width */
		size_t width = 0;
		if(__is_digit(*fmt)){
			width = __atoi(fmt++);
			__set_bit(flags, WIDTH_FLAG | PAD_FLAG | SPACE_FLAG);
		} else if (*fmt == '*') {
			int w = va_arg(vl, int);
			if(w < 0) {
				width = -w;
				__set_bit(flags, LEFT_FLAG);
			} else {
				width = w;
			}
			__set_bit(flags, WIDTH_FLAG | PAD_FLAG | SPACE_FLAG);
			fmt++;
		}

		/* check precision */
		size_t precision = 0;
		if(*fmt == '.'){
			fmt++;
			__set_bit(flags, PRECS_FLAG | PAD_FLAG | ZERO_FLAG);
			if(__is_digit(*fmt)){
				precision = __atoi(fmt++);
			} else if(*fmt == '*'){
				precision = va_arg(vl, int);
				fmt++;
			}
		}

		/* check length */
		switch(*fmt){
			case 'l':
				fmt++;
				if (*fmt == 'l'){
					__set_bit(flags, LLONG_FLAG);
					fmt++;
				} else {
					__set_bit(flags, LONG_FLAG);
				}
				break;

			case 'h':
				fmt++;
				if(*fmt == 'h') {
					__set_bit(flags, CHAR_FLAG);
					fmt++;
				}
				else {
					__set_bit(flags, SHORT_FLAG);
				}
				break;

			case 'j':
				fmt++;
				if(sizeof(intmax_t) == sizeof(long))
					__set_bit(flags, LONG_FLAG);
				else
					__set_bit(flags, LLONG_FLAG);
				break;

			case 'z':
				fmt++;
				if(sizeof(size_t) == sizeof(long))
					__set_bit(flags, LONG_FLAG);
				else
					__set_bit(flags, LLONG_FLAG);
				break;

			case 't':
				fmt++;
				if(sizeof(ptrdiff_t) == sizeof(long))
					__set_bit(flags, LONG_FLAG);
				else
					__set_bit(flags, LLONG_FLAG);
				break;
		}

		/* read actual specifier */
		size_t base = 10;
		size_t value = 0;
		const char *s = 0;
		void *p = 0;
		int *n = 0;
		char c = 0;

		switch(*fmt){
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
			case 'b':
				/* integer handling */
				switch(*fmt){
					case 'x': base = 16; break;
					case 'X': base = 2; break;
					case 'o': base = 8;  break;
					default : base = 10; break;
				}

				if(base == 10)
					__clear_bit(flags, HASH_FLAG);

				/* precision takes precedence */
				if(__is_set(flags, PRECS_FLAG))
					width = precision;

				/* formatting doesn't apply to decimal integers
				 * */
				if(*fmt != 'i' && *fmt != 'd'){
					__clear_bit(flags, SIGN_FLAG);
					__set_bit(flags, UNSIGN_FLAG);
				}

				if(__is_set(flags, LLONG_FLAG))
					value = va_arg(vl, long long);
				else if(__is_set(flags, LONG_FLAG))
					value = va_arg(vl, long);
				else
					value = va_arg(vl, int);

				chars_written += __print_integral(value, base,
						flags, width);
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
				for(; *s ;){
					__putchar(*s++);
					chars_written++;
				}
				fmt++;
				break;

			case 'p':
				p = va_arg(vl, void *);
				__set_bit(flags, UNSIGN_FLAG | HASH_FLAG);

				if(sizeof(void *) == sizeof(long))
					__set_bit(flags, LONG_FLAG);
				else
					__set_bit(flags, LLONG_FLAG);

				chars_written += __print_integral((ssize_t)p, 16,
						flags, width);
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
