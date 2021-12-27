#ifndef APOS_UAPI_H
#define APOS_UAPI_H

#include <apos/vmem.h>

/* syscall function type, let's start with four arguments and see where that
 * goes */
typedef vm_t (*sys_t)(vm_t a0, vm_t a1, vm_t a2, vm_t a3);

vm_t sys_req_mem(vm_t size, vm_t flags, vm_t u0, vm_t u1);
vm_t sys_req_pmem(vm_t paddr, vm_t size, vm_t flags, vm_t u0);
vm_t sys_req_fixmem(vm_t start, vm_t size, vm_t flags, vm_t u0);
vm_t sys_req_sharedmem(vm_t pid, vm_t start, vm_t size, vm_t flags);
vm_t sys_free_mem(vm_t start, vm_t u0, vm_t u1, vm_t u2);

vm_t sys_ipc_server(vm_t callback, vm_t u0, vm_t u1, vm_t u2);
vm_t sys_ipc_req(vm_t tgt_pid, vm_t d0, vm_t d1, vm_t d2);
vm_t sys_ipc_resp(vm_t tgt_pid, vm_t ret, vm_t u0, vm_t u1);

vm_t sys_fork(vm_t pid, vm_t u1, vm_t u2, vm_t u3);
vm_t sys_exec(vm_t pid, vm_t bin, vm_t argc, vm_t argv);
vm_t sys_signal(vm_t pid, vm_t signal, vm_t u0, vm_t u1);
vm_t sys_switch(vm_t pid, vm_t u0, vm_t u1, vm_t u2);
vm_t sys_sync(vm_t buf, vm_t size, vm_t u1, vm_t u2);

vm_t sys_conf(vm_t param, vm_t val, vm_t u0, vm_t u1);
vm_t sys_poweroff(vm_t type, vm_t u0, vm_t u1, vm_t u2);

#endif /* APOS_UAPI_H */
