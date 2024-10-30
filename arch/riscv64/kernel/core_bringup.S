#include "../kernel/csr.h"
#include "asm.h"

/* pic forces core_bringup to be placed in .got that we can access both from
 * direct mapped and virtual memory. Which is kind of strange, as core_bringup()
 * is fixed in place by the VM, so it could be possible to just load the address
 * directly into a register, but apparently that's annoying to do */
.option pic
.global riscv_bringup
riscv_bringup:
	/* immediately switch to virtual memory through argument in a1 */
	csrw satp, a1
	sfence.vma

	/* set scratch to zero in case we crash */
	csrw CSR_SSCRATCH, x0

	/* fetch the stack allocated to us at our hart index in smp_init_stacks */
	lui t0, %hi(smp_init_stacks)
	addi t0, t0, %lo(smp_init_stacks)
	slli t1, a0, RW_SHIFT
	add t0, t0, t1
	lr sp, 0(t0)
	mv tp, sp

	/* calculate actual address in kernelspace where we should jump to */
	/* a0 has RAM base */
	lui t0, %hi(core_bringup)
	addi t0, t0, %lo(core_bringup)

	/* jump to C to handle rest of bringup */
	jr t0
