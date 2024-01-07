#include "drivers/screen.h"
#include "kernel/cpu.h"
#include "kernel/gdt.h"
#include "kernel/kalloc.h"
#include "kernel/paging.h"
#include "kernel/spinlock.h"
#include "lib/debug.h"
#include "lib/string.h"

#include "kernel/proc.h"

// swtch.asm
void swtch(struct context **old, struct context *new);

// ptable implicitely zero-initialized: "Uninitialized variables with static
// duration […] are guaranteed to startout as zero"
// (https://c-faq.com/decl/initval.html).
struct ptable {
  struct spinlock lock;
  struct process proc[NPROC];
} ptable;

/** Next available PID value, incrementing overtime. */
static uint16_t nextpid = 1;

void process_init() {
    initlock(&ptable.lock, "ptable");

    // ptable and nextpid already initialized. Especially all processes are in
    // state UNUSED.
}

/**
 * Any new process "returns" to here, which in turn returns to the
 * return-from-trap part of `isr_handler_stub`, entering user mode
 * execution.
 */
static void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
}

/**
 * Find an UNUSED slot in the ptable and put it into INITIAL state. If all
 * slots are in use, return NULL.
 */
static struct process *
process_alloc(void)
{
    struct process *p;

    acquire(&ptable.lock);

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        if(p->state == UNUSED)
            goto found;

    release(&ptable.lock);
    warn("new_process: process table is full, no free slot");
    return NULL;

  found:
    p->state = INITIAL;
    p->pid = nextpid++;

    release(&ptable.lock);

    // Allocate kernel stack.
    if((p->kstack = (uint32_t)kalloc()) == 0){
        p->state = UNUSED;
        warn("new_process: failed to allocate kernel stack page");
        return NULL;
    }
    uint32_t sp = p->kstack + KSTACKSIZE;
    // We'll now allocate remaining process structures on the process stack.

    // TODO Trap handling

    sp -= sizeof *p->context;
    p->context = (struct context*)sp;
    memset(p->context, 0, sizeof *p->context);
    p->context->eip = (uint32_t)forkret;

    return p;
}


/**
 * Initialize the `init` process - put it in READY state in the process
 * table so the scheduler can pick it up.
 */
void
initproc_init(void)
{
    /** Get the embedded binary of `init.s`. */
    extern char _binary_kernel_initcode_start[];
    extern char _binary_kernel_initcode_size[];

    struct process *p = process_alloc();
    assert(p != NULL);

    if((p->pgdir = setupkvm()) == 0)
        panic("initproc: out of memory?");

    inituvm(p->pgdir, _binary_kernel_initcode_start, (int)_binary_kernel_initcode_size);

    p->sz = PGSIZE;

    // TODO trap frame initialization comes here

    strncpy(p->name, "initcode", sizeof(p->name) - 1);

    // In 13.-Abstraction-of-Process.md hux directly sets eip to
    // _binary___src_process_init_start as it doesn't copy the code to the
    // process memory with inituvm().
    p->context->eip = 0;

    // this assignment to p->state lets other cores
    // run this process. the acquire forces the above
    // writes to be visible, and the lock is also needed
    // because the assignment might not be atomic.
    acquire(&ptable.lock);

    /** Set process state to READY so the scheduler can pick it up. */
    p->state = READY;

    release(&ptable.lock);
}


static inline void proc_load_task_reg(uint16_t sel)
{
    __asm__ __volatile__ ( "ltr %0" : : "r" (sel) );
}

void
switchuvm(struct process *p)
{
  if(p == 0)
    panic("switchuvm: no process");
  if(p->kstack == 0)
    panic("switchuvm: no kstack");
  if(p->pgdir == 0)
    panic("switchuvm: no pgdir");

  pushcli();
  paging_switch_pgdir((void*)V2P(p->pgdir));  // switch to process's address space
  popcli();
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct process *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != READY)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      /* switchkvm(); */

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      /* c->proc = 0; */
    }
    release(&ptable.lock);

    break;
  }

}
