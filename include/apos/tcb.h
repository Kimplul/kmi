/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2021 - 2022, Kim Kuparinen < kimi.h.kuparinen@gmail.com > */

#ifndef APOS_TCB_H
#define APOS_TCB_H

/**
 * @file tcb.h
 * Process/thread handling.
 */

#include <apos/mem_regions.h>
#include <apos/caps.h>
#include <apos/types.h>
#include <arch/tcb.h> /* arch-specific data */

/**
 * Check if thread is process thread.
 *
 * @param t Thread to check.
 * @return \c true if thread is process thread, \c false otherwise.
 */
#define is_proc(t) (t->rid == t->tid)

/**
 * Check if thread is in RPC.
 *
 * @param t Thread to check.
 * @return \c true if thread is in RPC, \c false otherwise.
 */
#define is_rpc(t) (t->rid != t->pid)

/**
 * Get the process thread of current thread.
 *
 * @param t Thread whose effective process thread to get.
 * @return The process thread of the current thread.
 */
#define get_proc(t) (get_tcb(t->eid))

/**
 * Get the root process thread of current thread.
 *
 * @param t Thread whose root process thread to get.
 * @return The root process thread of the current thread.
 */
#define get_rproc(t) (get_tcb(t->rid))

/* forward declaration */
struct tcb;

/** Convenience structure for \ref tcb. */
struct tcb_ctx {
	/** Virtual address space of context. */
	struct vmem *vmem;

	/** Next thread in context. */
	struct tcb *next;

	/** Previous thread in context. */
	struct tcb *prev;
};

/** Enum for notification states. */
enum tcb_notify {
	/** Thread is free to be notified. */
	NOTIFY_WAITING = 0,

	/** Thread has notifcations queued. */
	NOTIFY_QUEUED,

	/** Thread is running notification handler. */
	NOTIFY_RUNNING,
};

/** Thread control block. Main way to handle threads. */
struct tcb {
	/** Execution continuation point. Important that it is first. */
	vm_t exec;

	/** Arch-specific data. */
	struct arch_tcbd tcbd;

	/** Memory mapping data. */
	struct mem_region_root sp_r;

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

	/** Thread ID. */
	id_t tid;

	/** \todo implement cpu_id to hardware cpu ID translation, first in
	 * riscv. */
	/** Cpu currently executing this thread. */
	id_t cpu_id;

	/** Address of callback function in servers. */
	vm_t callback;

	/** Capabilities of thread. */
	capflags_t caps;

	/** Address of this thread's stack base. */
	vm_t thread_stack;

	/** Address of this thread's stack top. */
	vm_t thread_stack_top;

	/** Current address of usable rpc stack. */
	vm_t rpc_stack;

	/** \todo Check if each thread should be allowed more than just one
	 * region of thread local storage. */
	/** Possible thread local storage. */
	vm_t thread_storage;

	/** Process context of thread. */
	struct tcb_ctx proc;

	/** RPC context of thread. */
	struct tcb_ctx rpc;

	/** Notifcation state of thread. */
	enum tcb_notify notify_state;

	/** Whether thread has gotten an IPI */
	bool ipi;
};

/**
 * Initialize thread control subsystem.
 */
void init_tcbs();

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
 * Attach a thread to an RPC context.
 *
 * Essentially inserts thread \c t into the process \c r, with access to the
 * same memory except for the RPC stack.
 *
 * @param r Process to attach to.
 * @param t Thread to attach.
 * @return \ref OK on success, \ref ERR_INVAL if pointers are the same.
 */
stat_t attach_rpc(struct tcb *r, struct tcb *t);

/**
 * Detach a thread from an RPC context.
 *
 * \see attach_rpc().
 *
 * @param r Process to detach from.
 * @param t Thread to detach.
 * @return \ref OK on success, \ref ERR_INVAL if pointers are the same.
 *
 * \todo Should probably check that thread exists in the process?
 */
stat_t detach_rpc(struct tcb *r, struct tcb *t);

/**
 * Attach a thread in a process context.
 *
 * @param r Process to attach to.
 * @param t Thread to attach.
 * @return \ref OK on success, \ref ERR_INVAL if pointers are the same.
 */
stat_t attach_proc(struct tcb *r, struct tcb *t);

/**
 * Detach a thread from a process context.
 *
 * @param r Process to detach from.
 * @param t Thread to detach.
 * @return \ref OK on success, \ref ERR_INVAL if pointers are the same.
 */
stat_t detach_proc(struct tcb *r, struct tcb *t);

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
 * Clone process context memory mappings.
 *
 * Essentially make sure all threads in the process have identical memory
 * mappings.
 *
 * @param p Process whose memory mappings to clone.
 * @return \ref OK on success, something else otherwise.
 * \todo Check up on return codes.
 */
stat_t clone_proc_maps(struct tcb *p);

/**
 * Clone RPC context memory mappings.
 *
 * \see clone_proc_maps().
 *
 * @param r Server whose memory mappings to clone to threads in RPC to it.
 * @return \ref OK on success, something else otherwise.
 * \todo Check up on return codes.
 */
stat_t clone_rpc_maps(struct tcb *r);

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
 * Save thread context for rpc call.
 *
 * @param t Thread whose context to save.
 */
void save_context(struct tcb *t);

/**
 * Load thread context from rpc call.
 *
 * @param t Thread whose context to restore.
 */
void load_context(struct tcb *t);

#endif /* APOS_TCB_H */
