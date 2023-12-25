#ifndef PROC_H
#define PROC_H

#include "kernel/paging.h"

/** Max number of processes at any time. */
#define NPROC 64

/** Each process has a kernel stack of one page. */
#define KSTACKSIZE PGSIZE


/**
 * Process context registers defined to be saved across switches.
 *
 * PC is the last member (so the last value on stack not being explicitly
 * popped in `context_switch()), because it will then be used as the
 * return address of the snippet.
 */
struct context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;   /** Frame pointer. */
    uint32_t eip;   /** Instruction pointer (PC). */
} __attribute__((packed));


enum process_state {
    UNUSED,     /** Indicates PCB slot unused. */
    INITIAL,    // EMBRYO in xv6
    READY,      // RUNNABLE
    RUNNING,
    BLOCKED,    // SLEEPING
    TERMINATED  // ZOMBIE
};

/** Process control block (PCB). */
struct process {
    char name[16];                   /** Process name */
    uint16_t pid;                    /** Process ID */
    struct context *context;         /** Registers context */
    enum process_state state;        /** Process state */
    pde_t *pgdir;                    /** Process page directory */
    uint32_t kstack;                 /** Beginning of kernel stack for this process */
    uint32_t sz;                     /** Size of process memory (bytes) */
    // ... (TODO)
};


// Per-CPU state
struct cpu {
  /* uchar apicid;                // Local APIC ID */
  struct context *scheduler;   // swtch() here to enter scheduler
  /* struct taskstate ts;         // Used by x86 to find stack for interrupt */
  /* struct segdesc gdt[NSEGS];   // x86 global descriptor table */
  /* volatile uint started;       // Has the CPU started? */
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct process *proc;        // The process running on this cpu or null
};

struct cpu* mycpu(void);

void cpu_init();


/** Extern the process table to the scheduler. */
extern struct ptable ptable;

void process_init();
void initproc_init(void);


#endif /* PROC_H */
