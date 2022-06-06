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
#define IDT_IRQ_BASE     32
#define IDT_IRQ_TIMER    (IDT_IRQ_BASE + 0)
#define IDT_IRQ_KEYBOARD (IDT_IRQ_BASE + 1)
#define IDT_IRQ_SIZE_MAX 48

#define IDT_DESCRIPTOR_X16_INTERRUPT  0x06
#define IDT_DESCRIPTOR_X16_TRAP       0x07
#define IDT_DESCRIPTOR_X32_TASK       0x05
#define IDT_DESCRIPTOR_X32_INTERRUPT  0x0E
#define IDT_DESCRIPTOR_X32_TRAP       0x0F
#define IDT_DESCRIPTOR_RING1          0x40
#define IDT_DESCRIPTOR_RING2          0x20
#define IDT_DESCRIPTOR_RING3          0x60
#define IDT_DESCRIPTOR_PRESENT        0x80

#define IDT_DESCRIPTOR_EXCEPTION      (IDT_DESCRIPTOR_X32_INTERRUPT | IDT_DESCRIPTOR_PRESENT)
#define IDT_DESCRIPTOR_EXTERNAL       (IDT_DESCRIPTOR_X32_INTERRUPT | IDT_DESCRIPTOR_PRESENT)
#define IDT_DESCRIPTOR_CALL           (IDT_DESCRIPTOR_X32_INTERRUPT | IDT_DESCRIPTOR_PRESENT | IDT_DESCRIPTOR_RING3)

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
