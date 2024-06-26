#include "drivers/screen.h"
#include "cpu.h"
#include "gdt.h"
#include "kalloc.h"
#include "paging.h"
#include "spinlock.h"
#include "syscall.h"
#include "lib/debug.h"
#include "lib/string.h"

#include "proc.h"

// swtch.asm
extern void swtch(struct context **old, struct context *new);

// isr.asm
extern void trapret(void);

// ptable implicitely zero-initialized: "Uninitialized variables with static
// duration […] are guaranteed to startout as zero"
// (https://c-faq.com/decl/initval.html).
struct ptable {
  struct spinlock lock;
  struct process proc[NPROC];
} ptable;

static struct process *initproc;

/** Next available PID value, incrementing overtime. */
static uint16_t nextpid = 1;


void process_init() {
    initlock(&ptable.lock, "ptable");

    // ptable and nextpid already initialized. Especially all processes are in
    // state UNUSED.

    isr_register(IDT_TRAP_SYSCALL, &syscall_handler);
}

/**
 * Any new process "returns" to here, which in turn returns to the
 * return-from-trap part of `isr_handler_stub`, entering user mode
 * execution.
 */
static void
enter_process(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  // Return to "caller", actually trapret (see allocproc).
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
    // We'll now allocate remaining process structures on the process kernel stack.

    /**
     * Leave room for the trap state. The initial context will be pushed right
     * below this trap state, with return address EIP pointing to `trapret`
     * (the return-from-trap part of `isr_handler_stub`). This way, the new
     * process, after context switch by the scheduler, automatically jumps into
     * user mode execution.
     */
    sp -= sizeof *p->tf;
    p->tf = (struct interrupt_state*)sp;

    // Set up new context to start executing at forkret,
    // which returns to trapret.
    sp -= sizeof(uint32_t);
    *(uint32_t*) sp = (uint32_t)trapret;

    sp -= sizeof *p->context;
    p->context = (struct context*)sp;
    memset(p->context, 0, sizeof *p->context);
    p->context->eip = (uint32_t)enter_process; // xv6's forkret

    return p;
}


/**
 * Initialize the `init` process - put it in RUNNABLE state in the process
 * table so the scheduler can pick it up.
 */
void
initproc_init(void)
{
    /** Get the embedded binary of `init.s`. */
    extern char _binary_user_init_start[];
    extern char _binary_user_init_size[];

    struct process *p = process_alloc();
    assert(p != NULL);

    initproc = p;
    if((p->pgdir = setupkvm()) == 0)
        panic("initproc: out of memory?");

    inituvm(p->pgdir, _binary_user_init_start, (int)_binary_user_init_size);

    p->sz = PGSIZE;

    memset(p->tf, 0, sizeof(*p->tf));
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;  // 0x1B
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;  // 0x23
    // es = ds in trapret
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->esp = PGSIZE;
    p->tf->eip = 0;  // beginning of init

    strncpy(p->name, "init", sizeof(p->name) - 1);

    // this assignment to p->state lets other cores
    // run this process. the acquire forces the above
    // writes to be visible, and the lock is also needed
    // because the assignment might not be atomic.
    acquire(&ptable.lock);

    /** Set process state to RUNNABLE so the scheduler can pick it up. */
    p->state = RUNNABLE;

    release(&ptable.lock);
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
enter_scheduler(void)   // sched() in xv6
{
  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  struct process *p = myproc();
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");

  int intena = mycpu()->intena;

  swtch(&p->context, mycpu()->scheduler);

  mycpu()->intena = intena;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(int status)
{
  struct process *p = myproc();

  if(p == initproc)
    warn("init exiting"); // TODO panic

  acquire(&ptable.lock);

  // Jump into the scheduler, never to return.
  p->xstate = status;
  p->state = ZOMBIE;
  enter_scheduler();
  panic("zombie exit");
}


// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct process* myproc(void) {
  pushcli();
  struct cpu *c = mycpu();
  struct process *p = c->proc;
  popcli();
  return p;
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
  gdt_set_entry(&mycpu()->gdt[SEG_TSS],
                /* base */ (uint32_t)&mycpu()->ts,
                /* limit */ sizeof(mycpu()->ts)-1,
                /* access */ SEG_PRESENT|SEG_RING0|SEG_TSS32_AVAIL, // 0x89 0b10001001
                /* flags */ 0); // 16-bit segment
  // SS0 and ESP0 are used by software interrupts.
  //
  // Reminder(see gdt_load.asm): selector in segmentation consists of
  // [15-3: idx, 2: table indicator, 1-0: requested privilege level].
  mycpu()->ts.ss0 = SEG_KDATA << 3;
  mycpu()->ts.esp0 = (uint32_t)p->kstack + KSTACKSIZE;
  // setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
  // forbids I/O instructions (e.g., inb and outb) from user space
  mycpu()->ts.iomb = (uint16_t) 0xFFFF;
  proc_load_task_reg(SEG_TSS << 3);
  paging_switch_pgdir((void*)V2P(p->pgdir));  // switch to process's address space
  popcli();
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
void
switchkvm(void)
{
  paging_switch_pgdir((pde_t*)V2P(kpgdir));  // switch to the kernel page table
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
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);

      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);

  }

}
