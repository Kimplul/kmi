#include <apos/types.h>
#include <apos/debug.h>

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

int __serial_tx_empty()
{
	return port->lsr & LSR_THRE;
}

void __serial_putchar(char c)
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

void dbg(const char *fmt, ...)
{
	/* do a proper implementation later, for now just dump stuff */
	while(*fmt){
		__serial_putchar(*fmt++);
	}
}

#endif /* DEBUG */
