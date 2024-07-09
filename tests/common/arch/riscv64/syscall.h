#ifndef KMI_TESTS_ARCH_RISCV64_SYSCALL_H
#define KMI_TESTS_ARCH_RISCV64_SYSCALL_H

#include <kmi/syscalls.h>
#include <kmi/attrs.h>

static inline struct sys_ret syscall(size_t n,
                                     long arg0, long arg1, long arg2, long arg3,
                                     long arg4,
                                     long arg5)
{
        register long a0 __asm__ ("a0") = arg0;
        register long a1 __asm__ ("a1") = arg1;
        register long a2 __asm__ ("a2") = arg2;
        register long a3 __asm__ ("a3") = arg3;
        register long a4 __asm__ ("a4") = arg4;
        register long a5 __asm__ ("a5") = arg5;

#define OUTPUTS "+r" (a0), "=r" (a1), "=r" (a2), "=r" (a3), "=r" (a4), "=r" (a5)

        if (n == 1)
                __asm__ volatile ("ecall" : OUTPUTS : "r" (a0));

        else if (n == 2)
                __asm__ volatile ("ecall" : OUTPUTS : "r" (a0), "r" (a1));

        else if (n == 3)
                __asm__ volatile ("ecall" : OUTPUTS : "r" (a0), "r" (a1),
                                  "r" (a2));

        else if (n == 4)
                __asm__ volatile ("ecall" : OUTPUTS
                                  : "r" (a0), "r" (a1), "r" (a2), "r" (a3));

        else if (n == 5)
                __asm__ volatile ("ecall" : OUTPUTS
                                  : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
                                  "r" (a4));

        else if (n == 6)
                __asm__ volatile ("ecall" : OUTPUTS
                                  : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
                                  "r" (a4), "r" (a5));

#undef OUTPUTS

        return (struct sys_ret){a0, a1, a2, a3, a4, a5};
}

#endif /* KMI_TESTS_ARCH_RISCV64_SYSCALL_H */
