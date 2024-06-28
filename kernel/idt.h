#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_SIZE_MAX                  256

/**
 * List of used interrupt numbers in this system. Other parts of the kernel
 * should refer to these macro names instead of using plain numbers.
 *   - 0 - 31 are ISRs for CPU-generated exceptions
 *   - >= 32 are mapped as custom device IRQs, so ISR 32 means IRQ 0, etc.
 */
// Processor-defined:
#define IDT_INT_DIVIDE      0      // divide error
#define IDT_INT_DEBUG       1      // debug exception
#define IDT_INT_NMI         2      // non-maskable interrupt
#define IDT_INT_BRKPT       3      // breakpoint
#define IDT_INT_OFLOW       4      // overflow
#define IDT_INT_BOUND       5      // bounds check
#define IDT_INT_ILLOP       6      // illegal opcode
#define IDT_INT_DEVICE      7      // device not available
#define IDT_INT_DBLFLT      8      // double fault
// #define IDT_INT_COPROC   9      // reserved (not used since 486)
#define IDT_INT_TSS        10      // invalid task switch segment
#define IDT_INT_SEGNP      11      // segment not present
#define IDT_INT_STACK      12      // stack exception
#define IDT_INT_GPFLT      13      // general protection fault
#define IDT_INT_PGFLT      14      // page fault
// #define IDT_INT_RES     15      // reserved
#define IDT_INT_FPERR      16      // floating point error
#define IDT_INT_ALIGN      17      // aligment check
#define IDT_INT_MCHK       18      // machine check
#define IDT_INT_SIMDERR    19      // SIMD floating point error

#define IDT_IRQ_BASE       32
// For these, use +IDT_IRQ_BASE to get the ISR.
#define IDT_IRQ_TIMER      0
#define IDT_IRQ_KEYBOARD   1
#define IDT_IRQ_COM1       4
#define IDT_IRQ_IDE1       14
#define IDT_IRQ_IDE2       15
#define IDT_IRQ_ERROR      19
#define IDT_IRQ_SIZE_MAX   48

#include "idt_defs.h"

#define IDT_DESCRIPTOR_X32_TASK       0x05
#define IDT_DESCRIPTOR_X16_INTERRUPT  0x06
#define IDT_DESCRIPTOR_X16_TRAP       0x07
#define IDT_DESCRIPTOR_X32_INTERRUPT  0x0E
#define IDT_DESCRIPTOR_X32_TRAP       0x0F
#define IDT_DESCRIPTOR_RING1          0x40
#define IDT_DESCRIPTOR_RING2          0x20
#define IDT_DESCRIPTOR_RING3          0x60
#define IDT_DESCRIPTOR_PRESENT        0x80

#define IDT_DESCRIPTOR_EXCEPTION      (IDT_DESCRIPTOR_X32_INTERRUPT | IDT_DESCRIPTOR_PRESENT)
#define IDT_DESCRIPTOR_EXTERNAL       (IDT_DESCRIPTOR_X32_INTERRUPT | IDT_DESCRIPTOR_PRESENT)
#define IDT_DESCRIPTOR_CALL           (IDT_DESCRIPTOR_X32_TRAP | IDT_DESCRIPTOR_PRESENT | IDT_DESCRIPTOR_RING3)

struct idt_entry {
  uint16_t base_lo;             // The lower 16 bits of the ISR's addresses
  uint16_t selector;            // The GDT segment selector that the CPU will load into CS before calling the ISR
  uint8_t  zero;                // Set to zero
  /*
    Type and attributes:
    - P (1 bit) Present. Must be set for the descriptor to be valid.
    - DPL (2 bits) CPU Privilege Levels allowed to access this interrupt via
    the INT instruction. Ignored for hardware interrupts.
    - 0 (1 bit) ignored
    - Gate type (4 bits)
  */
  uint8_t  type_attr;
  uint16_t base_hi;             // The higher 16 bits of the ISR's address
} __attribute__((packed));

// Not strictly needed now, but can be used later to reload the IDT.
struct idtr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

struct interrupt_state {
  uint32_t ds;
  uint32_t edi, esi, ebp, useless, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags, esp, ss;
} __attribute__((packed));

// Allow other modules to register an ISR.
typedef void (*isr_fn)(struct interrupt_state *);

void isr_register(uint8_t int_no, isr_fn handler);
void idt_init(void);

#endif /* IDT_H */
