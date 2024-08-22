/* SPDX-License-Identifier: copyleft-next-0.3.1 */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef KMI_TCB_H
#define KMI_TCB_H

/**
 * @file tcb.h
 * Process/thread handling.
 */

/* forward declaration */
struct tcb;

#include <kmi/orphanage.h>
#include <kmi/syscalls.h>
#include <kmi/regions.h>
#include <kmi/atomic.h>
#include <kmi/queue.h>
#include <kmi/types.h>
#include <kmi/caps.h>

#include <arch/tcb.h>

/**
 * Check if thread is process thread.
 *
 * @param t Thread to check.
 * @return \c true if thread is process thread, \c false otherwise.
 */
#define is_proc(t) ((t->rid == t->tid) && !orphan(t))

/**
 * Check if thread is in RPC.
 *
 * @param t Thread to check.
 * @return \c true if thread is in RPC, \c false otherwise.
 */
#define is_rpc(t) (!rpc_stack_empty(t->rpc_stack))

/**
 * Get the effective process thread of current thread.
 *
 * @param t Thread whose effective process thread to get.
 * @return The process thread of the current thread.
 */
#define get_proc(t) (get_tcb(t->eid))

/**
 * Alias for \ref get_proc(), to make it more obvious that we're accessing the
 * effective process. Might be useful in some situations.
 */
#define get_eproc(t) get_proc(t)

/**
 * Get the current process thread of current thread.
 *
 * @param t Thread whose current process thread to get.
 * @return The current process thread of the current thread.
 */
#define get_cproc(t) (get_tcb(t->pid))

/**
 * Get the root process thread of current thread.
 *
 * @param t Thread whose root process thread to get.
 * @return The root process thread of the current thread.
 */
#define get_rproc(t) (get_tcb(t->rid))

/** Convenience structure for \ref tcb. */
struct tcb_ctx {
	/** Virtual address space of context. */
	struct vmem *vmem;
};

/** State of thread. */
enum tcb_state {
	/** Thread is a zombie. */
	TCB_ZOMBIE = (1 << 0),

	/** Thread is an orphan. */
	TCB_ORPHAN = (1 << 1),
};

/** Wrapper around data for managing userspace virtual memory, defined here to
 * avoid loops */
struct uvmem {
	/** ID of owning thread. Zombie threads or orhaned threads may be moved
	 * to other virtual memories, but they still hold a 'backwards'
	 * reference to their original virtual memory. */
	id_t owner;

	/** Actual userspace virtual memory address space. */
	struct vmem *vmem;

	/** Region data for allocations within this address space. */
	struct mem_region_root region;
};

/** Thread control block. Main way to handle threads. */
struct tcb {
	/** Execution continuation point. Important that it is first. */
	vm_t exec;

	/**
	 * Address where to save registers.
	 * @note his address is the top of the register save structure.
	 */
	vm_t regs;

	/** Arch-specific data. */
	struct arch_tcbd arch;

	/** Memory mapping data. Only relevant in root thread. */
	struct uvmem uvmem;

	/** Address of callback function. */
	vm_t callback;

	/** Address of this thread's stack base. */
	vm_t thread_stack;

	/** Address of this thread's stack top. */
	vm_t thread_stack_size;

	/** Current address of usable rpc stack. */
	vm_t rpc_stack;

	/** Reference count to process. */
	long refcount;

	/** Process context of thread. */
	struct tcb_ctx proc;

	/** RPC context of thread. */
	struct tcb_ctx rpc;

	/**
	 * Effective process ID.
	 *
	 * This is the ID on which globally visible stuff should occur, such as
	 * memory allocations etc.
	 *
	 * When a thread is in an RPC, and a \ref SYS_IPC_FWD request occurs, the \c
	 * eid of the thread remains the same, whereas in a regular \c
	 * SYS_IPC_REQ the \c eid if replaced with the \ref pid of the
	 * process the thread is visiting.
	 */
	id_t eid;

	/**
	 * Actual process ID.
	 *
	 * This, along with \ref eid, creates the backbone of the IPC process ID
	 * handling.
	 */
	id_t pid;

	/**
	 * Root process ID.
	 *
	 * ID of the process that spawned the thread, and the process the thread
	 * should belong to when not in an RPC.
	 */
	id_t rid;

	/** Thread ID. @note all ids associated with threads use a signed type,
	 * but are always larger than zero. This is mirroring Linux behavior,
	 * and signed types are probably large enough. */
	id_t tid;

	/** \todo implement cpu_id to hardware cpu ID translation, first in
	 * riscv. */
	/** Cpu currently executing this thread. */
	id_t cpu_id;

	/** In which process to run notifications. Note that this is currently
	 * set in the syscall handlers due to the different rules for inheriting
	 * this id, might change in the future. */
	id_t notify_id;

	/** Currently waiting notifications. */
	enum notify_flag notify_flags;

	/** Capabilities of thread. */
	enum sys_cap caps;

	/** Queue that connects together threads waiting for an ipi */
	struct queue_head ipi_queue;

	/** Current state of thread. */
	enum tcb_state state;
};

/**
 * Initialize thread control subsystem.
 */
void init_tcbs();

/**
 * @return Maximum number of threads active at the same time.
 */
size_t max_tcbs();

/**
 * Destroy thread control subsystem.
 */
void destroy_tcbs();

/**
 * Create a new thread.
 *
 * If \c p is \c NULL, then a new process context is created for the thread.
 * Otherwise, the thread is inserted into \c p.
 *
 * The thread is allocated a virtual address space, as well as a kernel stack
 * and the \ref tcb structure itself with a unique thread ID. If in a new
 * process context, a new process address space is created as well.
 *
 * Userspace stack is allocated with \ref alloc_stack().
 *
 * \todo Thread local storage?
 *
 * @param p Process context within to create the thread.
 * @return Pointer to created \ref tcb.
 */
struct tcb *create_thread(struct tcb *p);

/**
 * Create a new process.
 *
 * Sets up a new thread in a new process context. If there is a parent thread,
 * its memory regions are copied.
 * \see create_thread().
 *
 * @param p Parent process.
 * @return Pointer to created \ref tcb.
 */
struct tcb *create_proc(struct tcb *p);

/**
 * Destroy a thread.
 *
 * Frees data associated with thread and frees up the thread ID.
 * At least currently does not allow \c t to be a process thread.
 *
 * \todo Other return values?
 *
 * @param t Thread to destroy.
 * @return \ref OK on success, \ref ERR_NOINIT if called without initializing
 * subsystem and \ref ERR_INVAL if called with a process thread.
 */
stat_t destroy_thread(struct tcb *t);

/**
 * Destroy a process.
 *
 * Frees all data associated with the process and destroys all threads within
 * it.
 *
 * @param p Process to destroy.
 * @return \ref OK on success, \ref ERR_NOINIT if called without initializing
 * subsystem and \ref ERR_INVAL if called without a process thread.
 */
stat_t destroy_proc(struct tcb *p);

/**
 * Get currently executing thread.
 *
 * @return Current \ref tcb.
 */
struct tcb *cur_tcb();

/**
 * Get thread currently running on cpu \p cpu_id.
 *
 * @param cpu_id CPU whose currently running thread to get.
 * @return \ref tcb running on cpu.
 */
struct tcb *cpu_tcb(id_t cpu_id);

/**
 * Get currently executing process.
 *
 * @return Effective process \ref tcb.
 */
struct tcb *cur_proc();

/**
 * Get currently executing process.
 *
 * @return Effective process \ref tcb.
 */
struct tcb *eff_proc();

/**
 * Set \c t as current \ref tcb.
 *
 * Also updates the current cpu id of the tcb.
 *
 * @param t Thread to mark as current.
 */
void use_tcb(struct tcb *t);

/**
 * Get \ref tcb corresponding to thread with ID \c tid.
 *
 * @param tid Thread ID.
 * @return Corresponding \ref tcb or \c NULL if not found.
 */
struct tcb *get_tcb(id_t tid);

/**
 * Allocate stacks for thread.
 *
 * Both user stack and RPC stack.
 *
 * @param t Thread whose stacks to allocate.
 * @return \ref OK on success, \ref ERR_OOMEM if out of memory.
 */
stat_t alloc_stack(struct tcb *t);

/**
 * Free thread stack.
 *
 * @param t Thread whose stack to free.
 */
void free_stack(struct tcb *t);

/**
 * Set address to jump to when returning to userspace.
 *
 * @param t Thread return address to set.
 * @param r Address to jump to.
 */
void set_return(struct tcb *t, vm_t r);

/**
 * Check whether \p t is currently running on some cpu.
 *
 * @param t \ref tcb to check.
 * @return \c true if it is running, \c false otherwise.
 */
bool running(struct tcb *t);

/**
 * Check whether \p t is actually dead but just kept around for resource
 * management.
 *
 * @param t \ref tcb to check.
 * @return \c true if \p t is a zombie, \c false otherwise.
 */
bool zombie(struct tcb *t);

/**
 * Add a reference to a process.
 * Instead of lists of threads that belong to a process, we give the process'
 * owning thread a reference counter. When a process is killed, a 'dead' bit is
 * set, and the thread that owns the process is unreferenced. All data
 * associated with the process can immediately be freed, but the tid is still
 * reserved until the reference count reaches zero.
 *
 * We have to make sure that all ways a process might be entered check that the
 * process is still alive, and unmapping pages causes other threads to update
 * their page tables as well. Then if a segfault happens, we can check if it was
 * due to being in a dead process. This is still largely TODO.
 *
 * @param p Process to reference.
 */
void reference_proc(struct tcb *p);

/**
 * Unreference a process.
 *
 * @param p Process to unreference.
 */
void unreference_proc(struct tcb *p);

#endif /* KMI_TCB_H */
