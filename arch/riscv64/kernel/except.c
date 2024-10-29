#include <kmi/types.h>
#include <kmi/panic.h>
#include <kmi/vmem.h>
#include <kmi/bkl.h>

void riscv_handle_exception(void *pc, void *addr, unsigned long id)
{
	switch (id) {
	case 12:
	case 13:
	case 15: {
		bkl_lock();
		handle_pagefault((vm_t)addr);
		bkl_unlock();
		break;
	}

	default:
		unhandled_panic(pc, addr, id);
	}
}
