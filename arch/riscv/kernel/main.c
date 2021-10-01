#include <apos/main.h>
#include <apos/debug.h>

void __main main(void *uart)
{
	dbg_init(uart, NS16550A);
	dbg("Hello from (hopefully) virtual memory!\n");
}
