#ifndef CPU_H
#define CPU_H

#include "drivers/acpi.h"
#include "kernel/gdt.h"

// Task state segment format
struct taskstate {
  uint32_t  link;               // Old ts selector
  uint32_t  esp0;               // Stack pointers and segment selectors
  uint16_t  ss0;                //   after an increase in privilege level
  uint16_t  padding1;
  uint32_t *esp1;
  uint16_t  ss1;
  uint16_t  padding2;
  uint32_t *esp2;
  uint16_t  ss2;
  uint16_t  padding3;
  void     *cr3;                // Page directory base
  uint32_t *eip;                // Saved state from last task switch
  uint32_t  eflags;
  uint32_t  eax;                // More saved state (registers)
  uint32_t  ecx;
  uint32_t  edx;
  uint32_t  ebx;
  uint32_t *esp;
  uint32_t *ebp;
  uint32_t  esi;
  uint32_t  edi;
  uint16_t  es;                 // Even more saved state (segment selectors)
  uint16_t  padding4;
  uint16_t  cs;
  uint16_t  padding5;
  uint16_t  ss;
  uint16_t  padding6;
  uint16_t  ds;
  uint16_t  padding7;
  uint16_t  fs;
  uint16_t  padding8;
  uint16_t  gs;
  uint16_t  padding9;
  uint16_t  ldt;
  uint16_t  padding10;
  uint16_t  t;                  // Trap on task switch
  uint16_t  iomb;               // I/O map base address
};

// Per-CPU state
struct cpu {
  uint8_t apicid;              // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  /* volatile bool started;       // Has the CPU started? */
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct process *proc;        // The process running on this cpu or null
};

extern struct cpu cpus[MAX_CPUS];
extern int ncpu;

/*
struct segdesc {
    uint16_t limit_lo;
  uint lim_15_0 : 16;  // Low bits of segment limit
    uint16_t base_lo;
  uint base_15_0 : 16; // Low bits of segment base address
    uint8_t base_mi;
  uint base_23_16 : 8; // Middle bits of segment base address
    uint8_t access;
  uint type : 4;       // Segment type (see STS_ constants)
  uint s : 1;          // 0 = system, 1 = application
  uint dpl : 2;        // Descriptor Privilege Level
  uint p : 1;          // Present
    uint8_t flags;
  uint lim_19_16 : 4;  // High bits of segment limit
  uint avl : 1;        // Unused (available for software use)
  uint rsv1 : 1;       // Reserved
  uint db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
  uint g : 1;          // Granularity: limit scaled by 4K when set
    uint8_t base_hi;
  uint base_31_24 : 8; // High bits of segment base address
};

#define SEG16(type, base, lim, dpl) (struct segdesc)  \
{ (lim) & 0xffff, (uint)(base) & 0xffff,              \

  ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \

  (uint)(lim) >> @16@, 0, 0, 1, @0@, (uint)(base) >> 24 }

#define SEG(type, base, lim, dpl) (struct segdesc)    \
{ ((lim) >> 12) & 0xffff, (uint)(base) & 0xffff,      \

  ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,   \

  (uint)(lim) >> @28@, 0, 0, 1, @1@, (uint)(base) >> 24 }
*/
struct cpu* mycpu(void);

void cpu_init();


#endif /* CPU_H */
